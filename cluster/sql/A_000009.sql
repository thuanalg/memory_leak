SELECT
	queueComplete.id AS 'recid',
	m.created_time AS 'calldate',
	m.source AS 'src',
	m.application_id,
	'ANSWERED' AS 'customer_status',
	queueComplete.`event` AS 'agent_status',
--	a.vcontext AS 'context',

 (	case when
--	ISNULL(queueComplete.data1),
	(queueComplete.data1 = 'NULL')
	then
	1
	else
	queueComplete.data1
	end
) AS 'time_in_queue',
 --a.agent_number,
 m.destination,
-- a.`name` AS 'agent_name',
 m.extension AS 'dst',
 queueComplete.data2 AS 'talk_time',
 m.call_note, 
 c.auto_dial_detail_id
-- au.phone_type
FROM
	queue_log queueComplete
JOIN (
	SELECT
		application_id,
		linkedid,
		auto_dial_detail_id
	FROM
		cdr
	WHERE
	--	calldate BETWEEN @fromDate
	--AND @toDate
	call_type = 'AUTO'
	GROUP BY
		linkedid
) AS c ON c.linkedid = queueComplete.callid
--JOIN auto_dial_detail au ON au.id = c.auto_dial_detail_id
--1
--AND au.created_time BETWEEN @fromDate
--AND @toDate
JOIN monitor_recording m ON queueComplete.callid = m.uniqueid
--AND m.created_time BETWEEN @fromDate
--AND @toDate
--JOIN agents a ON m.destination = a.agent_number
WHERE
--	queueComplete.time BETWEEN @fromDate
--AND @toDate
queueComplete.`event` IN (
	'COMPLETEAGENT',
	'COMPLETECALLER'
)
AND m.extension = SUBSTR(
	queueComplete.agent,
	5,
	LENGTH(queueComplete.agent)
)
-- AND a.vcontext = 'Collection-1'
-- AND a.agent_number = '5415'
AND m.call_note <> 'SYS_ERR'
ORDER BY
	m.created_time;
