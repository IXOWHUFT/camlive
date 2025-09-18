<?php
// notify.php
// Expects JSON POST: { "filename":"xxx.jpg", "url":"https://.../uploads/xxx.jpg", "weight": 62.3, "stream":"http://ip/stream", "timestamp": 1234567890 }
// Appends entry to data.json

$datFile = __DIR__ . '/data.json';
$input = file_get_contents("php://input");
if(!$input){
  http_response_code(400);
  echo json_encode(["error"=>"no input"]);
  exit;
}

$obj = json_decode($input, true);
if(!$obj){
  http_response_code(400);
  echo json_encode(["error"=>"invalid json"]);
  exit;
}

$entry = [
  "filename" => isset($obj['filename']) ? $obj['filename'] : '',
  "url" => isset($obj['url']) ? $obj['url'] : '',
  "weight" => isset($obj['weight']) ? floatval($obj['weight']) : null,
  "stream" => isset($obj['stream']) ? $obj['stream'] : '',
  "timestamp" => isset($obj['timestamp']) ? intval($obj['timestamp']) : time()
];

$data = [];
if(file_exists($datFile)){
  $content = file_get_contents($datFile);
  $data = json_decode($content, true) ?: [];
}

// prepend newest
array_unshift($data, $entry);

// keep last 500 entries
if(count($data) > 500) $data = array_slice($data, 0, 500);

file_put_contents($datFile, json_encode($data, JSON_PRETTY_PRINT));

header('Content-Type: application/json');
echo json_encode(["ok"=>true, "entry"=>$entry]);
?>