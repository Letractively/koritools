<?php
    
require("MailRu_RssFetcher.php");
require("SimpleTweet.php");
      
$MailRu_Rss_Url = "http://blogs.mail.ru/mail/%username%/?rss=1";
$Twitter_Username = "%twitter_username%";
$Twitter_Password = "%twitter_password%";
    
$RssFetcher = new CMailRu_RssFetcher($MailRu_Rss_Url);  
   
if( !$RssFetcher->GetNewItems($NewRssItems) )
{
    echo "<html><body>Error while fetching RSS feed.</body></html>";
    Exit;
}

$MaxCountOfTries = 2;
$TweetDelay = 1; // Секунд
$TweetDelayOnError = 5; //Секунд
$MaxTweetLen = 140; // Magic Twitter Number

if( count($NewRssItems)> 0)
{
    set_time_limit(count($NewRssItems)*$TweetDelayOnError*$MaxCountOfTries + 30);
    for($i = count($NewRssItems)-1; $i >= 0; $i--)
    {
        $Message = $NewRssItems[$i][$c_DescriptionTagName];

        $Message = preg_replace("#<a href=\"(.+)\".*>.*</a>#Ui", "$1", $Message); // Делаем из html-ссылок plain text
        //TODO: От остальных html тегов текст тоже надо чистить, т.к. иногда они все же просачиваются
        //$Message = iconv("windows-1251", "utf-8", $Message); //FIXME: Mail.ru всегда возвращает RSS в кодировке windows-1251 
        // После preg_replace кодировка волшебным образом меняется на utf-8
        if(mb_strlen($Message, "utf-8") > $MaxTweetLen ) 
        {
            $Message = mb_substr($Message, 0, $MaxTweetLen-1, "utf-8");
        }
        $AllOk = False;
        for($CountOfTries = 0; $CountOfTries<=$MaxCountOfTries; $CountOfTries++)
        {
            if(SimpleTweet($Message, $Twitter_Username, $Twitter_Password))
            {
                $AllOk = $RssFetcher->MarkItemsRead($NewRssItems[$i]);
                Break;
            }
            sleep($TweetDelayOnError);   
        }
        if(!$AllOk)
        {
            echo "<html><body>Error while tweeting.</body></html>";
            Exit;
        }
        sleep($TweetDelay);
    }
    echo "<html><body>That's ok. Check your twitter.</body></html>";
}   
else
{
    echo "<html><body>No new RSS items.</body></html>";
}
  

  
?>