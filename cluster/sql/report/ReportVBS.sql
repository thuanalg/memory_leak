-- Report Voice Blaster
-- Report detail từng cuộc gọi của VBS

SET @fromDate = '2017-10-29 00:00:00',
	@toDate = '2017-10-29 19:00:00';

SELECT
	c.calldate,
	c.src,
	c.auto_dial_id,
	MAX(duration) AS 'duration',
	MAX(c.billsec) AS 'billsec',
	c.disposition,
	c.answer_date,
	MAX(c.end_date) AS 'end_date',
	MAX(c.application_id) AS 'application_id',

IF (
	c.disposition = 'ANSWERED',

IF (
	(
		ISNULL(LENGTH(MAX(a.note)))
		OR LENGTH(MAX(a.note)) = ''
	),
	0,
	LENGTH(MAX(a.note))
),
 0
) AS 'count_text'
FROM
	cdr c
LEFT JOIN auto_dial_detail a ON c.auto_dial_detail_id = a.id
AND a.created_time BETWEEN @fromDate
AND @toDate
WHERE
	c.calldate BETWEEN @fromDate
AND @toDate
GROUP BY
	c.linkedid
ORDER BY
	c.calldate
