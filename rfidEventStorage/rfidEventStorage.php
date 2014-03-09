<?php
ini_set('display_errors', 1);
error_reporting(E_ALL);

require 'Slim/Slim.php';

\Slim\Slim::registerAutoloader();

require 'Views/jsonResponse.php';

class eventStorageDB extends SQLite3{
    function __construct(){
        $this->open('./db/test.db', SQLITE3_OPEN_READWRITE|SQLITE3_OPEN_CREATE);
    }
    function initTables(){
        $this->exec('DROP TABLE events');
        $this->exec('CREATE TABLE events (id STRING UNIQUE, owner STRING, source STRING, timestamp DATETIME)');
        //$this->exec("INSERT INTO events (id, owner, source, timestamp) VALUES ('a131', 'MyOwner', 'MySource', datetime('now'))");
    }

    function queryEvents($limit = null, $offset = null){
        $limitString = "";
        if ($limit !== null){
            if (!is_numeric($offset)){
                $offset = 0;
            }
            if (!is_numeric($limit)){
                $limit = 0;
            }
            $limitString = "LIMIT ".$offset.", ".$limit;
            var_dump($limitString);
        }
        return $this->query("SELECT * FROM events ".$limitString);
    }

    function queryEvent($id){
        return $this->query("SELECT * FROM events WHERE events.id = '$id'");
    }

    function insertEvent($owner, $source){
        $id = uniqid("", false);
        if ($this->exec("INSERT INTO events (id, owner, source, timestamp) VALUES ('$id', '$owner', '$source', datetime('now'))")){
            return $id;
        }
        else {
            return null;
        }
    }
}

$app = new \Slim\Slim(array('view' => new jsonResponse()));

$app->get('/init', function () {
    $db = new eventStorageDB();
    $db->initTables();
});

$app->get('/events', function () use ($app){
    $db = new eventStorageDB();
    $results = $db->queryEvents($app->request->params('limit'), $app->request->params('offset'));
    $retVal['href'] = $app->request->getUrl().$app->request->getPath();
    $retVal['events'] = array();
    while($row = $results->fetchArray(SQLITE3_ASSOC)){
        $retVal['events'][] = $row;
    }

    $app->render('jsonResponse.php', array('responseData' => $retVal));
});

$app->get('/event/:id', function($id) use($app){
    $db = new eventStorageDB();
    $results = $db->queryEvent($id);
    $retVal['href'] = $app->request->getUrl().$app->request->getPath();
    $retVal['event'] = $results->fetchArray(SQLITE3_ASSOC);
    $app->render('jsonResponse.php', array('responseData' => $retVal));
})->name('event');

$app->post('/events', function() use($app){
    $newEvent = json_decode($app->request->getBody());
    $db = new eventStorageDB();
    if (isset($newEvent->owner) && isset($newEvent->source)){
        $insertId = $db->insertEvent($newEvent->owner, $newEvent->source);
        if ($insertId === null){
            //Fehler
        }
        else {
            //Erzeugtes Event zurückliefern
            $results = $db->queryEvent($insertId);
            $retVal['href'] = $app->request->getUrl().$app->request->getPath();
            $retVal['event'] = $results->fetchArray(SQLITE3_ASSOC);
            $app->render('jsonResponse.php', array('responseData' => $retVal));
            $app->response->setStatus(201);
        }
    }
});

$app->run();

?>
