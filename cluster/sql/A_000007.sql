
SELECT 
        queueEnter.id AS 'recid',
        m.created_time AS 'calldate',
        m.source AS 'src',
        m.application_id,
        'ANSWERED' AS 'customer_status',
        queueComplete.`event` AS 'agent_status',
	-- a.vcontext AS 'context',
	 ( case when (queueComplete.data1 not null) then queueComplete.data1 else 1 end) as 'time_in_queue',
	 m.destination,
	-- a.`name` AS 'agent_name',
	 m.extension AS 'dst',
-- IF(INSTR(queueComplete.agent, a.fixed_extension)>0,queueComplete.data2,0) AS 'talk_time'
	 queueComplete.agent as 'tmp1',
	 queueComplete.data2 as 'tmp2',
	 m.call_note 
FROM
        queue_log queueEnter
LEFT JOIN (
        SELECT
                *
        FROM
                queue_log
        WHERE
        `event` IN (
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
        ) AS m ON queueEnter.callid = m.uniqueid
--	LEFT JOIN agents a ON m.destination = a.agent_number
WHERE
queueEnter.`event` = 'CONNECT' AND m.extension = SUBSTR(queueComplete.agent,5,LENGTH(queueComplete.agent))
AND m.call_note <> 'SYS_ERR'
ORDER BY
        m.created_time;
