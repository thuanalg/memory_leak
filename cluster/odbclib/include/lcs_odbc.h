#ifndef __LCS_ODBC_LIB_H__
#define __LCS_ODBC_LIB_H__
#ifndef LCSU64
	#define LCSU64	unsigned long long
#endif
#ifdef __cplusplus
	extern "C" {
#endif
	int lcs_whs_connect(char *server, int port);
	int lcs_whs_analysis(int conn, char *ana, LCSU64 from, LCSU64 to);
	int lcs_whs_close(int conn);
#ifdef __cplusplus
}
#endif
#endif
