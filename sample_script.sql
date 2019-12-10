create table if not exists glv_data(
time char not null,
sensorIndex char not null,
sensorName char not null,
value char not null
);

create table if not exists tsi_data(
time char not null,
sensorIndex char not null,
sensorName char not null,
value char not null
);

create table if not exists tsv_data(
time char not null,
sensorIndex char not null,
sensorName char not null,
value char not null
);

create table if not exists cooling_data(
time char not null,
sensorIndex char not null,
sensorName char not null,
value char not null
);

create table if not exists system_log(
time char not null,
reactionId char not null,
message char not null
);

create table if not exists sensors(
sensorIndex char not null,
sensorName char not null,
subsystem char not null,
minThreshold char not null,
maxThreshold char not null,
maxReactionId char not null,
minReactionId char not null
);

create table if not exists reactions(
reactionId char not null,
message char not null
);

create table if not exists system_info(
runId char not null,
startTime char not null,
endTime char not null
);
