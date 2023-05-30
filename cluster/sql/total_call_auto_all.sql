-- Total Call Auto All
SELECT 
 COUNT(1) totalCallsOutAuto, disposition
FROM
 cdr 
WHERE
 calldate >= '2017-10-29 08:05:00'
AND calldate <'2017-10-29 08:10:00'
AND auto_dial_detail_id nOT NULL
AND LENGTH(src) > 6 
AND LENGTH(dst) > 6 
GROUP BY disposition;
