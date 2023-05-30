-- Total Call Auto Viettel
SELECT
 COUNT(1) totalCallsOutAuto, disposition
FROM
 cdr
WHERE
auto_dial_detail_id not null
AND LENGTH(src) > 6
AND LENGTH(dst) > 6
AND (SUBSTR(dst,1,3) IN ('097','098','086') OR SUBSTR(dst,1,4) IN ('0163','0165','0166','0167','0168','0169','0162','0164'))
GROUP BY 
	disposition
order by
	disposition;
