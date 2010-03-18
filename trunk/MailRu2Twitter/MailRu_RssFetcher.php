<?php

$c_DescriptionTagName = "DESCRIPTION";
$c_GuidTagName        = "GUID";
$c_PubDateTagName     = "PUBDATE";

class CMailRu_RssFetcher
{
    #Parse cache
    private $Values;
    private $Tags;
    
    private $RssUrl;
    
    #Info about read items
    const c_StorageFileName = 'readitems.dat';
    private $StorageOpened = False;
    private $ReadItemsStorage_FileName;
    private $ReadItemsStorage;
    
    #Methods
    function __construct($RssUrl)
    {
        $this->RssUrl = $RssUrl;
    }
    
    private function ClearParseResult()
    {
        unset($this->Values);
        unset($this->Tags);    
    }
    
    private function AppendIthTagInfo(&$OutArray, $TagName, $i)
    {
        global $c_DescriptionTagName;

        if($TagName == $c_DescriptionTagName)
            $TagPos = $this->Tags[$TagName][$i+1];
        else
            $TagPos = $this->Tags[$TagName][$i];
        if($this->Values[$TagPos]["type"] != "complete")
        {
            return False;
        }
        $OutArray[$TagName] = $this->Values[$TagPos]["value"];      
        return True;
    }
    
    public function GetRssAsArray(&$Result)
    {
        global $c_DescriptionTagName, $c_GuidTagName, $c_PubDateTagName;
        
        if( empty($this->RssUrl) )
        {
            return False;
        }
        
        //$Rss = file_get_contents($this->RssUrl);
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $this->RssUrl);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, True); 
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, True);
        
        
        $Rss = curl_exec($ch);
        
        $Result = array();
        if( (curl_errno($ch) != 0) || empty($Rss) )
        {   
            curl_close($ch);
            return False;
        }
        curl_close($ch);
        $this->ClearParseResult();
        $Parser = xml_parser_create();
        xml_parse_into_struct($Parser, $Rss, $this->Values, $this->Tags);
        xml_parser_free($Parser);
        //print_r($Values);
        //print_r($Tags);
        $ItemsCount = count($this->Tags[$c_DescriptionTagName]) - 1;
        if( ($ItemsCount <= 0) || ($ItemsCount != count($this->Tags[$c_GuidTagName])) || ($ItemsCount !=count($this->Tags[$c_PubDateTagName])) )
        {
            $this->ClearParseResult();
            return False;
        }
        $AllOk = True;
        for($i=0; $i != $ItemsCount; $i++)
        {
            $Result[$i] = array();
            $AllOk =  $this->AppendIthTagInfo($Result[$i], $c_DescriptionTagName, $i) &&
                      $this->AppendIthTagInfo($Result[$i], $c_GuidTagName, $i) &&
                      $this->AppendIthTagInfo($Result[$i], $c_PubDateTagName, $i);
            if( !$AllOk)
            {
                Break;
            }
        }
        $this->ClearParseResult();
        if( $AllOk)
        {
            $this->FixDateFormat($Result);
            //TODO: Sort result array by date
        }
        return $AllOk;
    }
    
    private function FixDateFormat(&$RssItems)
    {
        global $c_PubDateTagName; 
        foreach($RssItems as $Key=>$RssItem)
        {
            $RssItems[$Key][$c_PubDateTagName] = strtotime($RssItem[$c_PubDateTagName]);
        }
    }
    
    private function CreateStorage()                           
    {
        $ReadedItemsStorage = array();
        $ReadedItemsStorage[$this->RssUrl] = array();
    }
    
    private function ValidateStorage()
    {
        if( empty($this->ReadedItemsStorage) || !is_array($this->ReadedItemsStorage) )   
        {
            return False;
        }
        foreach($this->ReadedItemsStorage as $Key=>$Value)
        {
            if(!is_string($Key) || !is_array($Value))
            {
                return False;
            }
        }
        return True;
    }  
    
    private function OpenStorage()
    {
        //TODO:
        if($this->StorageOpened)
        {
            return True;
        }
        $this->ReadedItemsStorage_FileName = self::c_StorageFileName;
        if( !file_exists($this->ReadedItemsStorage_FileName) )
        {
            $this->CreateStorage();       
        }
        else
        {
            $RawJsonData = file_get_contents($this->ReadedItemsStorage_FileName);
            $this->ReadedItemsStorage = json_decode($RawJsonData, True);
            
            if( !$this->ValidateStorage())
            {
                unset($this->ReadItemsStorage);
                print_r($this->ReadedItemsStorage);
                print_r($this->ReadedItemsStorage_FileName);
                
                return False;
            }
        }
        $this->StorageOpened = True;
        return True;
    }
    
    private function FlushStorage()
    {
        if(!$this->StorageOpened || !$this->ValidateStorage())
        {
            return False;
        }
        $RawJsonData = json_encode($this->ReadedItemsStorage);
        return file_put_contents($this->ReadedItemsStorage_FileName,$RawJsonData);
    }
    
    public function GetNewItems(&$Result)
    {
        if( !$this->OpenStorage() || !$this->GetRssAsArray($Result))
        {
            return False;
        }                                                                     
        global $c_GuidTagName, $c_PubDateTagName; 
        $LatestReadItemGuid = $this->ReadedItemsStorage[$this->RssUrl][$c_GuidTagName];
        $LatestReadPubDate  = $this->ReadedItemsStorage[$this->RssUrl][$c_PubDateTagName];
        if( empty($LatestReadItemGuid) || empty($LatestReadPubDate) )
        {
            return True;
        }
        $NeedToDelete = False;
        foreach($Result as $Key => $RssItem)
        {
            $NeedToDelete |= ($LatestReadItemGuid==$RssItem[$c_GuidTagName]) || ($LatestReadPubDate > $RssItem[$c_PubDateTagName]);
            if($NeedToDelete)    
            {
                unset($Result[$Key]);
            }
        }
        $Result = array_values($Result);
        return True;   
    }
    
    public function MarkItemsRead($LastReadItem)
    {
        global $c_GuidTagName, $c_PubDateTagName;
        if( empty($LastReadItem[$c_GuidTagName]) || empty($LastReadItem[$c_PubDateTagName]) )
        {
            return False;
        }
        $this->ReadedItemsStorage[$this->RssUrl][$c_GuidTagName] = $LastReadItem[$c_GuidTagName];
        $this->ReadedItemsStorage[$this->RssUrl][$c_PubDateTagName] = $LastReadItem[$c_PubDateTagName];
        return $this->FlushStorage();
    }
}

?>
