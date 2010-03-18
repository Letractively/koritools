<?php
    // Thanx to 2Coders
    function SimpleTweet($message, $username, $password)
    {
       $url = 'http://twitter.com/statuses/update.xml';

       $ch = curl_init();
       curl_setopt($ch, CURLOPT_URL, "$url");
       curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 2);
       curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
       curl_setopt($ch, CURLOPT_POST, 1);
       curl_setopt($ch, CURLOPT_POSTFIELDS, "status=$message");
       curl_setopt($ch, CURLOPT_USERPWD, "$username:$password");
       $result = curl_exec($ch);
       curl_close($ch);
       
       return !empty($result);
    }
?>