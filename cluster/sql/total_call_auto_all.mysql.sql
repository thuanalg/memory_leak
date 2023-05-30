SET @fromDate = '2017-10-29 00:00:00',
 @toDate = '2017-10-29 20:00:00';

-- Total Call Auto All
SELECT
 COUNT(1) totalCallsOutAuto, disposition
FROM
 cdr
WHERE
 calldate >= @fromDate
AND calldate < @toDate 
AND NOT ISNULL(auto_dial_detail_id)
AND LENGTH(src) > 6
AND LENGTH(dst) > 6
GROUP BY disposition;
