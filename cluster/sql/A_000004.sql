-- Total Call Auto Vina
SELECT
	COUNT(1) totalCallsOutAuto, disposition
FROM
	cdr
WHERE
	auto_dial_detail_id not null
	AND LENGTH(src) > 6
	AND LENGTH(dst) > 6
	AND (SUBSTR(dst,1,3) IN ('091','094','088') OR SUBSTR(dst,1,4) IN ('0123','0124','0125','0127','0129'))
GROUP BY 
	disposition
order by
	disposition;
