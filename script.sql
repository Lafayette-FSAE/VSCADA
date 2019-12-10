create table if not exists system_info(
starttime char not null,
endtime char not null,
recordindex char not null
);
create table if not exists system_log(
time char not null,
responseid char not null,
message char not null
);
create table if not exists sensor_data(
time char not null,
sensorindex char not null,
sensorname char not null,
rawdata char not null,
caldata char not null
);
create table if not exists sensors(
sensorindex char not null,
sensorname char not null,
minthreshold char not null,
maxthreshold char not null,
maxresponseid char not null,
minresponseid char not null,
calconstant char not null
);
create table if not exists GLV_sensors(
sensorindex char not null,
sensorname char not null
);
create table if not exists MOTOR_CONTROLLER_sensors(
sensorindex char not null,
sensorname char not null
);
create table if not exists TSI_sensors(
sensorindex char not null,
sensorname char not null
);
