//#define	WITH_HTML_WIDGET // Project-><project>Properites->C/C++->Preprocessor->PreprocessorDefinitions
						// to enable/disable html renderer, for <project>=AmbulantPlayer,libambulant_win32.
#ifdef	WITH_HTML_WIDGET
#include <string>

void* create_html_widget(std::string url, int left, int top, int width, int height);
int delete_html_widget(void* ptr);
void redraw_html_widget(void* ptr);
#endif // WITH_HTML_WIDGET
