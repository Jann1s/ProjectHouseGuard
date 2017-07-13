<?php
// Require the bundled autoload file - the path may need to change
// based on where you downloaded and unzipped the SDK
require __DIR__ . '/Twilio/autoload.php';

// Use the REST API Client to make requests to the Twilio REST API
use Twilio\Rest\Client;

// Your Account SID and Auth Token from twilio.com/console
$sid = 'ACe535a8c3b5c9f9fd2a4b75b058b76bc0';
$token = '8387ee1dd384a257ca2b2271650c7eaf';
$client = new Client($sid, $token);


//Getting Parameters:
$auth = $_GET['auth'];      //a hard-scripted auth code to be safe against unauthorized requests
$number = $_GET['number'];  //has to be this way: 4915120559108 (without beginning with a '+' or 00 -> only countrycode number)
$body = $_GET['body'];      //this is the sms content

if ($auth == 'EmmendDevHouseGuard') {

  // Use the client to do fun stuff like send text messages!
  $client->messages->create(
      // the number you'd like to send the message to
      '+' . $number,
      array(
          // A Twilio phone number you purchased at twilio.com/console
          'from' => '+3197004499281',
          // the body of the text message you'd like to send
          'body' => $body
      )
  );
}
else {
  echo "Warning: Not authorized to send sms.";
}
