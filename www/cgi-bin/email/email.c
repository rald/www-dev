#include "cgic.h"
#include "smtp.h"

#define MAIL_SERVER              "smtp.gmail.com"
#define MAIL_PORT                "587"
#define MAIL_CONNECTION_SECURITY SMTP_SECURITY_STARTTLS
#define MAIL_FLAGS               (SMTP_DEBUG         | \
                                  SMTP_NO_CERT_VERIFY) /* Do not verify cert. */
#define MAIL_CAFILE              NULL
#define MAIL_AUTH                SMTP_AUTH_PLAIN
#define MAIL_USER                "friaelagua@gmail.com"
#define MAIL_PASS                "vakb erou mpqj qkah"
#define MAIL_FROM                "friaelagua@gmail.com"
#define MAIL_FROM_NAME           "fria"
#define MAIL_SUBJECT             "greetings"
#define MAIL_BODY                "hello world"
#define MAIL_TO                  "thecoconutnutisagiantnut@gmail.com"
#define MAIL_TO_NAME             "friend"


void cgiInclude(FILE *fout,char *filename) {
	char line[256];
	FILE *fin=fopen(filename,"r");
	while(fgets(line,255,fin)) {
		fprintf(fout,"%s",line);
	}
}

int cgiMain() {

	char email_to[256];
	char email_subject[256];
	char email_body[1024];

	cgiHeaderContentType("text/html");

	cgiFormStringNoNewlines("email_to",email_to,sizeof(email_to));
	cgiFormStringNoNewlines("email_subject",email_subject,sizeof(email_subject));
	cgiFormString("email_body",email_body,sizeof(email_body));

	fprintf(cgiOut,
		"<form action='email.cgi' method='post'>"
		"To: <input name='email_to' type='email' value='%s'><br>"
		"Subject: <input name='email_subject' type='text' value='%s'><br>"
		"Body: <textarea name='email_body'>%s</textarea><br>"
		"<input name='send' type='submit' value='send'><br>"
		"</form>",email_to,email_subject,email_body
	);

	if(cgiFormSubmitClicked("send")==cgiFormSuccess) {

		if(strlen(email_to)==0) { fprintf(cgiOut,"email to required.<br>"); return 1; }

		if(strlen(email_subject)==0) { fprintf(cgiOut,"email subject required.<br>"); return 1; }

		if(strlen(email_body)==0) { fprintf(cgiOut,"email body required.<br>"); return 1; }

	  struct smtp *smtp;
	  int rc;
	  rc = smtp_open(MAIL_SERVER,
	                 MAIL_PORT,
	                 MAIL_CONNECTION_SECURITY,
	                 MAIL_FLAGS,
	                 MAIL_CAFILE,
	                 &smtp);
	  rc = smtp_auth(smtp,
	                 MAIL_AUTH,
	                 MAIL_USER,
	                 MAIL_PASS);
	  rc = smtp_address_add(smtp,
	                        SMTP_ADDRESS_FROM,
	                        MAIL_FROM,
	                        MAIL_FROM_NAME);
	  rc = smtp_address_add(smtp,
	                        SMTP_ADDRESS_TO,
	                        email_to,
	                        MAIL_TO_NAME);
	  rc = smtp_header_add(smtp,
	                       "Subject",
	                       email_subject);
	  rc = smtp_mail(smtp,
	                 email_body);
	  rc = smtp_close(smtp);

	  if(rc != SMTP_STATUS_OK) {
	    fprintf(cgiOut, "smtp failed: %s<br>\n", smtp_status_code_errstr(rc));
	    return 1;
	  }

	fprintf(cgiOut,"email sent<br>\n");

	}

	return 0;
}
