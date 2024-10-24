-- Report Notconnect
-- Đây là report những cuộc gọi mà hệ thống call cho khách hàng nhưng không kết nối được.
-- Report này đi theo từng campaign cụ thể nên ngoài filter theo thời gian còn có thêm trường auto dial id.

SET @fromDate = '2017-10-29 00:00:10',
 @toDate = '2017-10-29 19:00:00',
 @autoDialId = '2946';

SELECT

IF (
	c.disposition <> "ANSWERED",

IF (
	LENGTH(a.application_id) = 16,
	"VPBFC",
	"VPBANK"
),
 ""
) AS FINANCIER_ID,
 a.application_id AS APPL_ID,

IF (
	c.disposition <> "ANSWERED",
	a.customer_id,
	""
) AS CUST_ID,

IF (
	c.disposition <> "ANSWERED",

IF (
	a.skill = '1',
	'PDS00000',
	'ES100000'
),
 ""
) AS USER_ID,

IF (
	c.disposition <> "ANSWERED",
	DATE_FORMAT(c.calldate, "%d/%m/%Y"),
	""
) AS ACTION_DATE,

IF (
	c.disposition <> "ANSWERED",
	DATE_FORMAT(c.calldate, "%H:%i"),
	""
) AS ACTION_TIME,

IF (
	c.disposition = "NO ANSWER",

IF (a.skill = '1', "PD_NKP", "NKP"),

IF (
	c.disposition = "ANSWERED",
	"ANSWERED",

IF (a.skill = '1', "PD_NAB", "NAB")
)
) AS ACTION_CODE,

IF (
	c.disposition <> "ANSWERED",
	"PHONE",
	""
) AS CONTACT_MODE,

IF (
	c.disposition <> "ANSWERED",
	"NOBODY",
	""
) AS PERSON_CONTACTED,

IF (
	c.disposition <> "ANSWERED",
	"TMPADD",
	""
) AS PLACE_CONTACTED,

IF (
	c.disposition <> "ANSWERED",
	"VND",
	""
) AS CURRENCY,
 "" AS ACTION_AMOUNT,
 "" AS NEXT_ACTION_DATE,
 "" AS NEXT_ACTION_TIME,

IF (
	c.disposition <> "ANSWERED",
	"MOBC",
	""
) AS REMINDER_MODE,

IF (
	c.disposition <> "ANSWERED",

IF (a.skill = '1', "PDS", "ECS"),
 ""
) AS CONTACTED_BY,

IF (
	c.disposition <> "ANSWERED",
	CONCAT(
		'Uptrail_Auto_',
		a.phone_number,
		'_',

	IF (
		c.disposition = "NO ANSWER",

	IF (a.skill = '1', "PD_NKP", "NKP"),

IF (a.skill = '1', "PD_NAB", "NAB")
	)
	),
	""
) AS REMARKS
FROM
	auto_dial_detail a
JOIN cdr c ON a.id = c.auto_dial_detail_id
AND c.calldate BETWEEN @fromDate
AND @toDate
WHERE
	a.created_time BETWEEN @fromDate
AND @toDate
-- AND a.auto_dial_id IN (@autoDialId)
AND c.call_type = 'AUTO';
