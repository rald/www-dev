<!doctype html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>POCS: Register Student</title>
    <style>
        * { font-family: monospace; }
    </style>
</head>
<body>

<?php
    include("links.php");
?>

<hr>

<?php
require_once("errors.php");
require_once("connect.php");

if (isset($_POST['id'])) {
    $id = (int)$_POST['id'];

    $stmt = $db->prepare("DELETE FROM schedule WHERE id = :id");
    $stmt->bindValue(":id", $id, SQLITE3_INTEGER);

    if ($stmt->execute()) {
        echo "Record deleted";
    } else {
        echo "Error deleting record";
    }

    $stmt->close();
}

echo "<form method='post' action='" . htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES) . "'>";

$sql = "
    SELECT schedule.id AS id, student_id, nick, subject_id, description, dayOfWeek, beginTime, endTime
    FROM schedule
    JOIN student ON student_id = student.id
    JOIN subject ON subject_id = subject.id
";

$res = $db->query($sql);

if ($res && $res->numColumns() > 0) {
    echo "<table border='1'>";
    echo "<tr>
            <th>ID</th>
            <th>Student&nbsp;ID</th>
            <th>Nick</th>
            <th>Subject&nbsp;ID</th>
            <th>Description</th>
            <th>Day</th>
            <th>Begin&nbsp;Time</th>
            <th>End&nbsp;Time</th>
            <th>Action</th>
          </tr>";

    while ($row = $res->fetchArray(SQLITE3_ASSOC)) {
        $beginTime = date('g:i A', strtotime($row['beginTime']));
        $endTime = date('g:i A', strtotime($row['endTime']));

        echo "<tr>
                <td>" . htmlspecialchars($row['id']) . "</td>
                <td>" . htmlspecialchars($row['student_id']) . "</td>
                <td>" . htmlspecialchars($row['nick']) . "</td>
                <td>" . htmlspecialchars($row['subject_id']) . "</td>
                <td>" . htmlspecialchars($row['description']) . "</td>
                <td>" . htmlspecialchars($row['dayOfWeek']) . "</td>
                <td>" . htmlspecialchars($beginTime) . "</td>
                <td>" . htmlspecialchars($endTime) . "</td>
                <td>
                    <button type='submit' name='id' value='" . htmlspecialchars($row['id']) . "' onclick='return confirm(\"Are you sure?\")'>Delete</button>
                </td>
              </tr>";
    }

    echo "</table>";
} else {
    echo "No records";
}

echo "</form>";

$db->close();
?>

	</body>
</html>

