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
require_once "errors.php";
require_once "connect.php";

$stu = '';
$nck = '';
$msg = '';

if (
    $_SERVER['REQUEST_METHOD'] === 'POST' &&
    isset($_POST['submit']) &&
    $_POST['submit'] === 'Add'
) {
    $stu = trim($_POST['stu'] ?? '');
    $nck = trim($_POST['nck'] ?? '');

    if ($stu !== '' && $nck !== '') {
        $stmt = $db->prepare("SELECT id FROM student WHERE id = :id");
        $stmt->bindValue(':id', $stu, SQLITE3_TEXT);
        $res = $stmt->execute();

        if ($res && $res->fetchArray(SQLITE3_NUM) === false) {
            $stmt = $db->prepare("INSERT INTO student (id, nick) VALUES (:id, :nick)");
            $stmt->bindValue(':id', $stu, SQLITE3_TEXT);
            $stmt->bindValue(':nick', $nck, SQLITE3_TEXT);

            if ($stmt->execute()) {
                $msg = "Record added";
            } else {
                $msg = "Error adding record";
            }
        } else {
            $msg = "Student ID already exists";
        }
    } else {
        $msg = "Please fill in all fields";
    }
}
?>

<?php if ($msg !== '') echo htmlspecialchars($msg); ?>

<form method="post" action="<?= htmlspecialchars($_SERVER['PHP_SELF']) ?>">
    <table>
        <tr>
            <td>Student ID</td>
            <td><input name="stu" type="text" value="<?= htmlspecialchars($stu) ?>" pattern="^[A-L]\d{4}$" size="5" maxlength="5"></td>
        </tr>
        <tr>
            <td>Student Nickname</td>
            <td><input name="nck" type="text" value="<?= htmlspecialchars($nck) ?>" size="20" maxlength="20"></td>
        </tr>
        <tr>
            <td align="right" colspan="2"><input name="submit" type="submit" value="Add"></td>
        </tr>
    </table>
</form>

</body>
</html>
<?php
$db->close();
?>
