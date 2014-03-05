<?php
switch ($_SERVER['REQUEST_METHOD']){
    case 'GET':
        echo json_encode("GET REQUEST");
        break;
    case 'POST':
        echo "POST_REQUEST";
        $json = file_get_contents('php://input');
        $obj = json_decode($json);
        var_dump($obj);
        echo "My name is ".$obj->name;
        break;
}
?>
