# The KMime headers names are prepended with kmime_ to avoid name clashes.
# We can't use ecm_generate_headers if the CamelCase header doesn't match the filename
set(KMime_HEADERS
  Content
  ContentIndex
  DateFormatter
  HeaderParsing
  Headers
  MDN
  Message
  NewsArticle
  Util
  Types
)

install(FILES
        ${KMime_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KMime/KMime COMPONENT Devel
)
