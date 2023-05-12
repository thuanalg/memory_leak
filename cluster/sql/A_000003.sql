-- Total Call Auto Mobi
SELECT
	COUNT(1) totalCallsOutAuto, disposition
FROM
	cdr
WHERE
	auto_dial_detail_id not null
	AND LENGTH(src) > 6
	AND LENGTH(dst) > 6
	AND (SUBSTR(dst,1,3) IN ('090','093','089') OR SUBSTR(dst,1,4) IN ('0121','0122','0126','0128','0120'))
GROUP BY 
	disposition 
order by 
	disposition;
