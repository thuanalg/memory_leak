SET @fromDate = '2017-10-29 00:00:00',
 @toDate = '2017-10-29 22:05:00';

SELECT
    c.application_id,
    c.calldate,
    c.src,
    c.dst,
    c.disposition,

IF (
    NOT ISNULL(q.data2)
    AND disposition = 'ANSWERED',
    q.data2 + c.duration,
    c.duration
) AS 'duration',

IF (
    NOT ISNULL(q.data2)
    AND disposition = 'ANSWERED',
    q.data2,
    0   
) AS 'billsec',
 q.`event` AS 'connect result'
FROM
    cdr c
LEFT JOIN (
    SELECT
        *   
    FROM

        queue_log
    WHERE
        time >= @fromDate
    AND @toDate > time
    AND time >= @fromDate
    AND @toDate > time
    AND `event` IN (
        'COMPLETEAGENT',
        'COMPLETECALLER',
        'ABANDON',
        'EXITWITHTIMEOUT'
    )
) q ON c.linkedid = q.callid
WHERE
    c.calldate >= @fromDate
AND @toDate > calldate
AND c.dcontext = 'auto-dial-out';
