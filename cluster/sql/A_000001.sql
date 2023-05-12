-- Total Call Auto All
SELECT 
 COUNT(1) totalCallsOutAuto, disposition
FROM
 cdr 
WHERE
auto_dial_detail_id nOT NULL
AND LENGTH(src) > 6 
AND LENGTH(dst) > 6 
GROUP BY 
	disposition
order by 
	disposition;
