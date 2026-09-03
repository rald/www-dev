<?php

$db = new SQLite3('pantasya.db');
$results = $db->query('SELECT * FROM schedule');

$data = [];
while ($row = $results->fetchArray(SQLITE3_ASSOC)) {
    $data[] = $row;
}

?>

<script>
    let data=<?php echo json_encode($data, JSON_PRETTY_PRINT);?>;
    console.log(data);
</script>

