<?php
// list_uploads.php
$dir = __DIR__ . '/uploads/';
$baseUrl = (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] === 'on' ? 'https://' : 'http://') . $_SERVER['HTTP_HOST'] . dirname($_SERVER['REQUEST_URI']);
$baseUrl = rtrim($baseUrl, '/') . '/uploads/';
$dataFile = __DIR__ . '/data.json';
$data = [];
if(file_exists($dataFile)){
    $data = json_decode(file_get_contents($dataFile), true) ?: [];
}

// map filename to metadata
$meta = [];
foreach($data as $d){
    if(!empty($d['filename'])) $meta[$d['filename']] = $d;
}

$out = [];
if(is_dir($dir)){
    $files = array_values(array_diff(scandir($dir), array('.', '..')));
    foreach($files as $f){
        $path = $dir . $f;
        if(is_file($path)){
            $item = [
                'name' => $f,
                'url' => $baseUrl . rawurlencode($f),
                'mtime' => filemtime($path),
                'weight' => null,
                'stream' => ''
            ];
            if(isset($meta[$f])){
                $item['weight'] = isset($meta[$f]['weight']) ? $meta[$f]['weight'] : null;
                $item['stream'] = isset($meta[$f]['stream']) ? $meta[$f]['stream'] : '';
                // prefer timestamp from metadata if present
                if(isset($meta[$f]['timestamp'])) $item['mtime'] = $meta[$f]['timestamp'];
            }
            $out[] = $item;
        }
    }
}
header('Content-Type: application/json');
echo json_encode(array_values($out));
?>