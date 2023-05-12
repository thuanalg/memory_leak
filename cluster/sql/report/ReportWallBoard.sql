-- Report Wallboard
-- Đây là report genaral theo agent, tổng số connect, tổng talktime...
-- Report này dùng show wallboard.

SET @fromDate = '2017-10-29 00:00:00',
	@toDate = '2017-10-29 19:00:00';

SELECT
	context,
	agent_number,
	agent_name,
	COUNT(agent_number) AS total_connect,
	SUM(talk_time) AS total_talk_time,
	SUM(is_ptp) AS total_ptp,
	TRUNCATE (SUM(is_ptp) / COUNT(*) * 100, 2) AS ptp_percent
FROM
(
SELECT
	a.vcontext AS 'context',
			a.agent_number,
			a.`name` AS 'agent_name',
		IF (
			INSTR(
				queueComplete.agent,
				a.fixed_extension
			) > 0,
			queueComplete.data2,
			0
		) AS 'talk_time',

	IF (m.call_note LIKE '%PTP%', 1, 0) AS is_ptp
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
) AS queueComplete ON queueEnter.callid = queueComplete.callid
AND queueEnter.agent = queueComplete.agent
LEFT JOIN (
	SELECT
		*
	FROM
		monitor_recording
	WHERE
		created_time BETWEEN @fromDate
	AND @toDate
) AS m ON queueEnter.callid = m.uniqueid
LEFT JOIN agents a ON m.destination = a.agent_number
WHERE
	queueEnter.time BETWEEN @fromDate
AND @toDate
AND queueEnter.`event` = 'CONNECT'
AND m.extension = SUBSTR(
	queueComplete.agent,
	5,
	LENGTH(queueComplete.agent)
)
AND m.call_note <> 'SYS_ERR'
AND m.call_note NOT IN ('NAB','NKP','WES','IGN5','NCP')
) AS temp
GROUP BY
	temp.agent_number
ORDER BY
	SUM(talk_time) DESC,
	COUNT(agent_number) DESC,
	(
		SUM(is_ptp) / COUNT(agent_number)
	) DESC,
	SUM(is_ptp) DESC;
