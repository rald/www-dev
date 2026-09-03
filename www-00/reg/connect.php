<?php

$dbname = "pantasya.db";

try {
    $db = new SQLite3($dbname);
} catch (Exception $e) {
    die("DB error: " . $e->getMessage());
}

if (!$db) {
    die("DB failed to open.");
}

?>
