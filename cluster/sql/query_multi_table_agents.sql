SET @fromDate = '2017-10-29 00:30:00', @toDate='2017-10-29 20:35:00';

SELECT 
        queueEnter.id AS 'recid',
        m.created_time AS 'calldate',
        m.source AS 'src',
        m.application_id,
        'ANSWERED' AS 'customer_status',
        queueComplete.`event` AS 'agent_status',
 IF(queueComplete.data1 IS NULL,1,queueComplete.data1) AS 'time_in_queue',
 m.extension AS 'dst',
 m.call_note,
 m.destination
-- a.agent_number,
-- a.`name` AS 'agent_name',
-- a.vcontext AS 'context',
-- IF(INSTR(queueComplete.agent, a.fixed_extension)>0,queueComplete.data2,0) AS 'talk_time'
FROM
        queue_log queueEnter
LEFT JOIN (
        SELECT
                *
        FROM
                queue_log
        WHERE
                time BETWEEN @fromDate
        AND @toDate
        AND `event` IN (
                'COMPLETEAGENT',
                'COMPLETECALLER',
                'TRANSFER'
        )
) AS queueComplete ON queueEnter.callid = queueComplete.callid AND queueEnter.agent = queueComplete.agent
LEFT JOIN (
        SELECT
                *
        FROM
                monitor_recording
        WHERE
                created_time BETWEEN @fromDate
        AND @toDate
) AS m ON queueEnter.callid = m.uniqueid
--	LEFT JOIN agents a ON m.destination = a.agent_number
WHERE
        queueEnter.time BETWEEN @fromDate
AND @toDate
AND queueEnter.`event` = 'CONNECT' AND m.extension = SUBSTR(queueComplete.agent,5,LENGTH(queueComplete.agent))
AND m.call_note <> 'SYS_ERR'
ORDER BY
        m.created_time;
