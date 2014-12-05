create table data (
       dew_p float,
       heat_idx_c float,
       humidity float,
       lux float,
       temp_c float,
       itime int,
       stamp timestamp without time zone default now()
);

create index stamp_idx on data (stamp);
