-- ReportVBS.sql

select 
calldate,
src,
auto_dial_id,
max(duration) as 'duration',
max(billsec) as 'billsec',
disposition,
answer_date,
max(end_date) as 'end_date',
max(application_id) as 'application_id',
case when (disposition="ANSWERED") then min(auto_dial_detail_id) else "0" end AS 'count_text'
from cdr 
group by linkedid order by calldate;
