
SELECT
    c.application_id,
    c.calldate,
    c.src,
    c.dst,
    c.disposition,

(	case when (q.data2 not null AND disposition = 'ANSWERED')
	then
    	q.data2 + c.duration
	else
    	c.duration
	end
) AS 'duration',

(	case when ( q.data2 not null AND disposition = 'ANSWERED')
	then
    	q.data2
	else
    	0   
	end
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
    `event` IN (
        'COMPLETEAGENT',
        'COMPLETECALLER',
        'ABANDON',
        'EXITWITHTIMEOUT'
    )
) q ON c.linkedid = q.callid
WHERE
c.dcontext = 'auto-dial-out';
