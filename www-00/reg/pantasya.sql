drop table if exists student;
drop table if exists subject;
drop table if exists schedule;



create table student (
	id varchar(5) not null primary key,
	nick varchar(20) not null
);



create table subject (
	id varchar(5) not null primary key,
	description varchar(200)
);



create table schedule (
	id integer not null primary key,
	student_id varchar(5) not null,
	subject_id varchar(5) not null,
	dayOfWeek varchar(1) not null,
	beginTime time not null,
	endTime time  not null,
	foreign key (student_id) references student(id) on delete cascade,
	foreign key (subject_id) references subject(id) on delete cascade
);



insert into student(id,nick) values
('I2200','Phoebe'),
('I2201','Kristan'),
('I2202','Venn'),
('I2203','Shi'),
('I2204','Xee'),
('I2205','Rachel'),
('I2206','Itami'),
('I2207','Lyzel'),
('I2208','Kenneth'),
('I2209','Ferd'),
('I2210','Timothy'),
('I2211','Yner'),
('I2212','John'),
('I2213','Johnben'),
('I2214','Ry'),
('I2215','Marlon'),
('I2216','April'),
('I2217','Hendrix'),
('B2300','Brake^t'),
('B2301','Pattu'),
('C2300','Zeyn'),
('D2300','Jane'),
('H2300','Lloyd'),
('I2300','Andrew'),
('J2300','Phol'),
('J2301','Sheen'),
('E2500','Rald');



insert into subject(id,description) values
('CLP','C Language Programming'),
('CPP','C++ Programming'),
('JAV','Java Programming'),
('PYT','Python Programming'),
('DSA','Data Structures and Algorithms'),
('HTM','Hypertext Markup Language'),
('CSS','Cascading Style Sheets'),
('JAS','Javascript Programming'),
('PHP','PHP Hypertext Preprocessor'),
('SQL','Structured Query Language');



insert into schedule(student_id,subject_id,dayOfWeek,beginTime,endTime) values
('E2500','CPP','F','13:00','14:30'),
('E2500','JAV','F','14:30','16:00');


