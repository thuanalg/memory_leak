SET @fromDate = '2017-10-29 06:00:00',
 @toDate = '2017-10-29 19:59:00';

SELECT
    DATE(calldate),
    HOUR (calldate),
    COUNT(*) AS 'totalCall',
    SUM(IF(hangup_cause=58,1,0)) AS 'totalFullChannel',
    SUM(IF((SUBSTR(dst,1,3) IN ('097','098','086','096') OR SUBSTR(dst,1,4) IN ('0163','0165','0166','0167','0168','0169','0162','0164')),1,0)) AS 'ViettelAuto',
    SUM(IF((hangup_cause = 58 AND (SUBSTR(dst,1,3) IN ('097','098','086','096') OR SUBSTR(dst,1,4) IN ('0163','0165','0166','0167','0168','0169','0162','0164'))),1,0)) AS 'ViettelFullChannel',
    SUM(IF((SUBSTR(dst,1,3) IN ('090','093','089') OR SUBSTR(dst,1,4) IN ('0121','0122','0126','0128','0120')),1,0)) AS 'MobiAuto',
    SUM(IF((hangup_cause = 58 AND (SUBSTR(dst,1,3) IN ('090','093','089') OR SUBSTR(dst,1,4) IN ('0121','0122','0126','0128','0120'))),1,0)) AS 'MobiFullChannel',
    SUM(IF((SUBSTR(dst,1,3) IN ('091','094','088') OR SUBSTR(dst,1,4) IN ('0123','0124','0125','0127','0129')),1,0)) AS 'VinaAuto',
    SUM(IF((hangup_cause = 58 AND (SUBSTR(dst,1,3) IN ('091','094','088') OR SUBSTR(dst,1,4) IN ('0123','0124','0125','0127','0129'))),1,0)) AS 'Vina'
    
FROM
 cdr
WHERE
 calldate BETWEEN @fromDate
AND @toDate
AND NOT ISNULL(auto_dial_detail_id)
AND LENGTH(src) > 6
AND LENGTH(dst) > 6 GROUP BY DATE(calldate), HOUR (calldate);
