drop table if exists queue_log; CREATE TABLE queue_log LIKE backup_queue_log;
INSERT queue_log SELECT * FROM backup_queue_log;
drop table if exists monitor_recording ; 
CREATE TABLE monitor_recording LIKE backup_monitor_recording;  
INSERT monitor_recording SELECT * FROM backup_monitor_recording;
drop table if exists cdr; 
CREATE TABLE cdr LIKE backup_cdr;  
INSERT cdr SELECT * FROM backup_cdr;

drop table if exists auto_dial_wrapup_history; 
CREATE TABLE auto_dial_wrapup_history LIKE backup_auto_dial_wrapup_history;  
INSERT auto_dial_wrapup_history SELECT * FROM backup_auto_dial_wrapup_history;

drop table if exists auto_dial_detail; 
CREATE TABLE auto_dial_detail LIKE backup_auto_dial_detail;  
INSERT auto_dial_detail SELECT * FROM backup_auto_dial_detail;
