<html>
	<head>
		<meta name="viewport" content="width=device-width, initial-scale=1.0" />
		<title>POCS: Registration</title>
		<style>
			* {
				font-family: monospace;
			}
		</style>
	</head>
	<body>

<?php
    include("links.php");
?>

<hr>

<?php
require_once("errors.php");
require_once("connect.php"); // must define $db = new SQLite3(...)

function convertTimeToMinutes($time) {
    list($hours, $minutes) = explode(':', $time);
    return ((int)$hours * 60) + (int)$minutes;
}

function conflicts($inputSchedule, $existingSchedules) {
    $conflicts = [];
    $inputDay = $inputSchedule['dow'];
    $inputStart = convertTimeToMinutes($inputSchedule['beg']);
    $inputEnd = convertTimeToMinutes($inputSchedule['end']);

    foreach ($existingSchedules as $schedule) {
        if ($schedule['dow'] === $inputDay) {
            $existingStart = convertTimeToMinutes($schedule['beg']);
            $existingEnd = convertTimeToMinutes($schedule['end']);

            if ($inputStart < $existingEnd && $existingStart < $inputEnd) {
                $conflicts[] = $schedule;
            }
        }
    }
    return $conflicts;
}

$hasConflict = false;
$showAdd = false;

$stu = '';
$sbj = '';
$dow = '';
$beg = '';
$end = '';

if (
    isset($_POST['stu'], $_POST['sbj'], $_POST['dow'], $_POST['beg'], $_POST['end'])
) {
    $stu = trim($_POST['stu']);
    $sbj = trim($_POST['sbj']);
    $dow = trim($_POST['dow']);
    $beg = trim($_POST['beg']);
    $end = trim($_POST['end']);

    if (isset($_POST['submit']) && $_POST['submit'] === 'Add') {
        $stmt = $db->prepare("
            INSERT INTO schedule (student_id, subject_id, dayOfWeek, beginTime, endTime)
            VALUES (:stu, :sbj, :dow, :beg, :end)
        ");

        if ($stmt) {
            $stmt->bindValue(':stu', $stu, SQLITE3_TEXT);
            $stmt->bindValue(':sbj', $sbj, SQLITE3_TEXT);
            $stmt->bindValue(':dow', $dow, SQLITE3_TEXT);
            $stmt->bindValue(':beg', $beg, SQLITE3_TEXT);
            $stmt->bindValue(':end', $end, SQLITE3_TEXT);

            $result = $stmt->execute();
            if ($result) {
                echo "Record added";
                $result->finalize();
            } else {
                echo "Error adding record: " . $db->lastErrorMsg();
            }
        } else {
            echo "Prepare failed: " . $db->lastErrorMsg();
        }
    }

    if (isset($_POST['submit']) && $_POST['submit'] === 'Check') {
        $stmt = $db->prepare("SELECT id FROM student WHERE id = :stu");
        if ($stmt) {
            $stmt->bindValue(':stu', $stu, SQLITE3_TEXT);
            $res = $stmt->execute();

            if ($res && $res->fetchArray(SQLITE3_ASSOC)) {
                $res->finalize();

                if (convertTimeToMinutes($beg) < convertTimeToMinutes($end)) {
                    $sch = [];
                    $stmt2 = $db->prepare("
                        SELECT id, student_id, subject_id, dayOfWeek, beginTime, endTime
                        FROM schedule
                    ");

                    if ($stmt2) {
                        $res2 = $stmt2->execute();
                        if ($res2) {
                            while ($row = $res2->fetchArray(SQLITE3_ASSOC)) {
                                $sch[] = [
                                    'idn' => $row['id'],
                                    'stu' => $row['student_id'],
                                    'sbj' => $row['subject_id'],
                                    'dow' => $row['dayOfWeek'],
                                    'beg' => $row['beginTime'],
                                    'end' => $row['endTime']
                                ];
                            }
                            $res2->finalize();
                        }
                    }

                    $inp = [
                        'stu' => $stu,
                        'sbj' => $sbj,
                        'dow' => $dow,
                        'beg' => $beg,
                        'end' => $end
                    ];

                    $cnf = conflicts($inp, $sch);
                    if (count($cnf) > 0) {
                        echo "<table border>";
                        echo "<caption><b>Conflict Schedule</b></caption>";
                        echo "<tr><th>ID</th><th>Student&nbsp;ID</th><th>Subject&nbsp;ID</th><th>Day</th><th>Begin&nbsp;Time</th><th>End&nbsp;Time</th></tr>";
                        foreach ($cnf as $row) {
                            echo "<tr>";
                            echo "<td>" . htmlspecialchars($row["idn"]) . "</td>";
                            echo "<td>" . htmlspecialchars($row["stu"]) . "</td>";
                            echo "<td>" . htmlspecialchars($row["sbj"]) . "</td>";
                            echo "<td>" . htmlspecialchars($row["dow"]) . "</td>";
                            echo "<td>" . date("g:i A", strtotime($row["beg"])) . "</td>";
                            echo "<td>" . date("g:i A", strtotime($row["end"])) . "</td>";
                            echo "</tr>";
                        }
                        echo "</table>";
                        $showAdd = false;
                    } else {
                        echo "No conflict schedule";
                        $showAdd = true;
                    }
                } else {
                    echo "Invalid Time";
                }
            } else {
                echo "Invalid Student ID";
            }

            if ($res) {
                $res->finalize();
            }
        } else {
            echo "Prepare failed: " . $db->lastErrorMsg();
        }
    }
}

echo "<form method='post' action='" . htmlspecialchars($_SERVER['PHP_SELF']) . "'>";
echo "<table>";

echo "<tr>";
echo "<td>Student ID</td>";
echo "<td><input name='stu' type='text' value='" . htmlspecialchars($stu) . "' pattern='^[A-L]\\d{4}$' size='5' maxlength='5' required></td>";
echo "</tr>";

echo "<tr>";
$stmt = $db->prepare("SELECT id, description FROM subject");
if ($stmt) {
    $res = $stmt->execute();
    echo "<td>Select&nbsp;Subject</td>";
    echo "<td><select name='sbj' required>";
    echo "<option value=''></option>";
    if ($res) {
        while ($row = $res->fetchArray(SQLITE3_ASSOC)) {
            $selected = ($row['id'] == $sbj) ? 'selected' : '';
            echo "<option value='" . htmlspecialchars($row['id']) . "' $selected>" .
                 htmlspecialchars($row['id']) . " --- " .
                 htmlspecialchars($row['description']) .
                 "</option>";
        }
        $res->finalize();
    }
    echo "</select></td>";
}
echo "</tr>";

echo "<tr>";
$dows = array("D", "M", "T", "W", "H", "F", "S");
echo "<td>Select&nbsp;Day</td>";
echo "<td><select name='dow' required>";
echo "<option value=''></option>";
foreach ($dows as $d) {
    echo "<option value='" . htmlspecialchars($d) . "' " . ($dow == $d ? 'selected' : '') . ">" . htmlspecialchars($d) . "</option>";
}
echo "</select></td></tr>";

echo "<tr><td>Begin&nbsp;Time</td><td><input name='beg' type='time' value='" . htmlspecialchars($beg) . "' required></td></tr>";
echo "<tr><td>End&nbsp;Time</td><td><input name='end' type='time' value='" . htmlspecialchars($end) . "' required></td></tr>";
echo "<tr><td colspan='2' align='right'><input name='submit' type='submit' value='Check'></td></tr>";

if ($showAdd) {
    echo "<tr><td colspan='2' align='right'><input name='submit' type='submit' value='Add'></td></tr>";
}

echo "</table>";
echo "</form>";
?>

	</body>
</html>
