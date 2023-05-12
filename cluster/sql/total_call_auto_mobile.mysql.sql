SET @fromDate = '2017-10-29 07:00:00',
 @toDate = '2017-10-29 20:10:00';
-- Total Call Auto Mobi
SELECT
 COUNT(1) totalCallsOutAuto, disposition
FROM
 cdr
WHERE
 calldate BETWEEN @fromDate
AND @toDate
AND NOT ISNULL(auto_dial_detail_id)
AND LENGTH(src) > 6
AND LENGTH(dst) > 6
AND (SUBSTR(dst,1,3) IN ('090','093','089') OR SUBSTR(dst,1,4) IN ('0121','0122','0126','0128','0120'))
GROUP BY disposition;
