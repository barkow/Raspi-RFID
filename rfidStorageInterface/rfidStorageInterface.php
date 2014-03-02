<?php
class rfidDaemonConnector {
    private $host = "localhost";
    private $port = "6378";
    private $socket;
    private $connected = false;
    public function isConnected(){
        return $this->connected;
    }
    public function connect(){
        $this->socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
        socket_set_option($this->socket, SOL_SOCKET, SO_RCVTIMEO, array('sec' => 0, 'usec' => 500000));
        if (!socket_connect($this->socket, $this->host, $this->port)){
            socket_close($this->socket);
            return false;
        }
        socket_set_block($this->socket);
        $this->connected = true;
        return true;
    }
    public function disconnect(){
        if ($this->connected){
            socket_close($this->socket);
        }
    }
    public function readTag(){
        if ($this->connected){
            $tagChar = "";
            $tagValue = "";
            $readFailed = false;
            while (!$readFailed && $tagChar != "\n"){
                $tagChar = @socket_read($this->socket, 1, PHP_BINARY_READ);
                if ($tagChar == false) {
                    //Wenn '' zurückgeliefert wird, besteht keine Verbindung zum Daemon mehr
                    if ($tagChar === '') {
                        $this->connected = false;
                    }
                    $readFailed = true;
                }
                else {
                    $tagValue = $tagValue.$tagChar;
                }
            }
            if ($readFailed){
                return NULL;
            }
            else {
                return $tagValue;
            }
        }
    }
}

class rfidStorageConnector{
    private $restURL = "http://localhost/rfidEventStorage/API";
    private $sourceName = "TestSource";
    public function addEvent($tagID){
        //POST an REST API Senden
        echo "REST API POST:".PHP_EOL;
        echo "Source:".$this->sourceName.PHP_EOL;
        echo "Timestamp:"."generated on server".PHP_EOL;
        echo "Owner:".$tagID.PHP_EOL;
        //Rückgabe auswerten.
        return true;
    }
}

class userSignalling{
    private $RESETSTATUSTIME = 10;
    public function showTagDetected(){
        echo "User Signal DETECTED".PHP_EOL;
        $this->lastChange = time();
    }
    public function showTagNotAccepted(){
        echo "User Signal NOT_ACCEPTED".PHP_EOL;
        $this->lastChange = time();
    }
    public function showTagAccepted(){
        echo "User Signal ACCEPTED".PHP_EOL;
        $this->lastChange = time();
    }
    public function showTagIdle(){
        echo "User Signal IDLE".PHP_EOL;
        $this->lastChange = time();
    }
    public function cyclic(){
        $now = time();
        if ($now - $this->lastChange > $this->RESETSTATUSTIME){
            $this->showTagIdle();
        }
    }
    private $lastChange = 0;
}

//TODO:
//Verbindung mit Daemon über Socket aufbauen
//Bei empfangenen RFID Daten Infos an REST API schicken
$rfidDaemon = new rfidDaemonConnector();
$rfidStorage = new rfidStorageConnector();
$userSignal = new userSignalling();
$userSignal->showTagIdle();
while(1){
    //Wenn keine Verbindung zum Daemon besteht, Verbindung jetzt herstellen
    if(!$rfidDaemon->isConnected()){
        //Wenn Verbindung nicht hergestellt werden kann, 10s warten, bevor neuer versuch unternommen wird
        if (!$rfidDaemon->connect()){
            Sleep(2);
        }
    }
    else{
        //Neuen Tag auslesen
        $newTag = $rfidDaemon->readTag();
        //Prüfen, ob ein gültiger Tag erkannt wurde
        if ($newTag != NULL) {
            //Erkennen eines Tags signalisieren
            $userSignal->showTagDetected();
            //Erfolg des Hinzufügens eines neuen Events signalisieren
            if ($rfidStorage->addEvent($newTag)){
                //Event wurde erfolgreich hinzugefügt
                $userSignal->showTagAccepted();
            }
            else {
                $userSignal->showTagNotAccepted();
            }
        }

    }
    $userSignal->cyclic();
}
$rfidDaemon->disconnect();
?>
