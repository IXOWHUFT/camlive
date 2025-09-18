<?php
// upload.php
// expects POST raw binary (image/jpeg)
// Saves to uploads/ and returns JSON {"url":"https://yourdomain.com/uploads/xxx.jpg","filename":"xxx.jpg"}

$uploadsDir = __DIR__ . '/uploads/';
if(!is_dir($uploadsDir)) mkdir($uploadsDir, 0755, true);

$raw = file_get_contents("php://input");
if(!$raw || strlen($raw) < 50) {
  http_response_code(400);
  echo json_encode(["error"=>"no data or file too small"]);
  exit;
}

$filename = 'photo_' . time() . '_' . bin2hex(random_bytes(4)) . '.jpg';
$path = $uploadsDir . $filename;
file_put_contents($path, $raw);

// determine public URL - adjust if behind reverse proxy
$scheme = (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] !== 'off') ? 'https' : 'http';
$host = $_SERVER['HTTP_HOST'];
$public_url = $scheme . '://' . $host . '/uploads/' . $filename;

header('Content-Type: application/json');
echo json_encode(["url"=>$public_url,"filename"=>$filename]);
?>