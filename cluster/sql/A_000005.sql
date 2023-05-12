SELECT
    DATE(calldate),
	strftime ("%H",calldate) as ghour,
    COUNT(*) AS 'totalCall',
    SUM( case when (hangup_cause=58) then 1 else 0 end  ) AS 'totalFullChannel',

    SUM( case when ((SUBSTR(dst,1,3) IN ('097','098','086','096') OR SUBSTR(dst,1,4) IN ('0163','0165','0166','0167','0168','0169','0162','0164')))
		then 1 else 0 end
	) AS 'ViettelAuto',
    SUM(  case when  ((hangup_cause = 58 AND (SUBSTR(dst,1,3) IN ('097','098','086','096') 
		OR SUBSTR(dst,1,4) 
		IN ('0163','0165','0166','0167','0168','0169','0162','0164'))) )  then 1 else 0 end) AS 'ViettelFullChannel',
    SUM( case  when ((SUBSTR(dst,1,3) IN ('090','093','089') 
		OR SUBSTR(dst,1,4) 
		IN ('0121','0122','0126','0128','0120'))) then 1 else 0 end ) AS 'MobiAuto',
    SUM( case when ((hangup_cause = 58 AND (SUBSTR(dst,1,3) IN ('090','093','089') 
		OR SUBSTR(dst,1,4) IN ('0121','0122','0126','0128','0120')))) then 1 else 0 end) AS 'MobiFullChannel',
    SUM( case when ((SUBSTR(dst,1,3) IN ('091','094','088') OR SUBSTR(dst,1,4) 
		IN ('0123','0124','0125','0127','0129'))) then 1 else 0 end  ) AS 'VinaAuto',
    SUM( case when ((hangup_cause = 58 AND (SUBSTR(dst,1,3) IN ('091','094','088') OR SUBSTR(dst,1,4) 
		IN ('0123','0124','0125','0127','0129')))) then 1 else 0 end  ) AS 'Vina'
    
FROM
 cdr
WHERE
-- calldate BETWEEN '2017-10-29 11:00:00'
--AND '2017-10-29 11:05:00'
--AND auto_dial_detail_id not null

	auto_dial_detail_id not null
	AND LENGTH(src) > 6
	AND LENGTH(dst) > 6 
GROUP BY 
	DATE(calldate),
	ghour;
--AND LENGTH(dst) > 6 GROUP BY DATE(calldate), HOUR (calldate);
