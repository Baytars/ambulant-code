#define	WITH_HTML_WIDGET // comment out to disable html renderer
#ifdef	WITH_HTML_WIDGET
#include <string>

void* create_html_widget(std::string url, int left, int top, int width, int height);
int delete_html_widget(void* ptr);
#endif // WITH_HTML_WIDGET
