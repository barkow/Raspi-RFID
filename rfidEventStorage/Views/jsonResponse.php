<?php
class jsonResponse extends \Slim\View{
    public function render($template, $data = NULL) {
        return json_encode($this->data['responseData']);
    }
}
?>
