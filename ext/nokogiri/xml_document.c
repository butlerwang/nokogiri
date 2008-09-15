#include <xml_document.h>

static void dealloc(xmlDocPtr doc)
{
  xmlFreeDoc(doc);
}

/*
 * call-seq:
 *  serialize
 *
 * Serialize this document
 */
static VALUE serialize(VALUE self)
{
  xmlDocPtr doc;
  xmlChar *buf;
  int size;
  Data_Get_Struct(self, xmlDoc, doc);

  xmlDocDumpMemory(doc, &buf, &size);
  VALUE rb_str = rb_str_new((char *)buf, (long)size);
  free(buf);
  return rb_str;
}

/*
 * call-seq:
 *  root=
 *
 * Set the root element on this document
 */
static VALUE set_root(VALUE self, VALUE root)
{
  xmlDocPtr doc;
  xmlNodePtr new_root;

  Data_Get_Struct(self, xmlDoc, doc);
  Data_Get_Struct(root, xmlNode, new_root);

  xmlDocSetRootElement(doc, new_root);
  return root;
}

/*
 * call-seq:
 *  root
 *
 * Get the root node for this document.
 */
static VALUE root(VALUE self)
{
  xmlDocPtr doc;
  Data_Get_Struct(self, xmlDoc, doc);

  xmlNodePtr root = xmlDocGetRootElement(doc);

  if(!root) return Qnil;
  return Nokogiri_wrap_xml_node(root);
}

static VALUE read_memory( VALUE klass,
                          VALUE string,
                          VALUE url,
                          VALUE encoding,
                          VALUE options )
{
  const char * c_buffer = StringValuePtr(string);
  const char * c_url    = (url == Qnil) ? NULL : StringValuePtr(url);
  const char * c_enc    = (encoding == Qnil) ? NULL : StringValuePtr(encoding);
  int len               = RSTRING(string)->len ;

  xmlInitParser();
  xmlDocPtr doc = xmlReadMemory(c_buffer, len, c_url, c_enc, NUM2INT(options));
  VALUE rb_doc = Data_Wrap_Struct(klass, NULL, dealloc, doc);
  doc->_private = (void *)rb_doc;
  return rb_doc;
}

static VALUE new(int argc, VALUE *argv, VALUE klass)
{
  VALUE version;
  if(rb_scan_args(argc, argv, "01", &version) == 0)
    version = rb_str_new2("1.0");

  xmlDocPtr doc = xmlNewDoc((xmlChar *)StringValuePtr(version));
  VALUE rb_doc = Data_Wrap_Struct(klass, NULL, dealloc, doc);
  doc->_private = (void *)rb_doc;
  return rb_doc;
}

/*
 *  call-seq:
 *    substitute_entities_set bool)
 *
 *  Set the global XML default for substitute entities.
 */
static VALUE substitute_entities_set(VALUE self, VALUE value)
{
    xmlSubstituteEntitiesDefault(NUM2INT(value));
    return Qnil ;
}

/*
 *  call-seq:
 *    substitute_entities_set bool)
 *
 *  Set the global XML default for load external subsets.
 */
static VALUE load_external_subsets_set(VALUE self, VALUE value)
{
    xmlLoadExtDtdDefaultValue = NUM2INT(value);
    return Qnil ;
}

void init_xml_document()
{
  VALUE m_nokogiri  = rb_const_get(rb_cObject, rb_intern("Nokogiri"));
  VALUE m_xml       = rb_const_get(m_nokogiri, rb_intern("XML"));
  VALUE klass       = rb_const_get(m_xml, rb_intern("Document"));

  rb_define_singleton_method(klass, "read_memory", read_memory, 4);
  rb_define_singleton_method(klass, "new", new, -1);
  rb_define_singleton_method(klass, "substitute_entities=", substitute_entities_set, 1);
  rb_define_singleton_method(klass, "load_external_subsets=", load_external_subsets_set, 1);

  rb_define_method(klass, "root", root, 0);
  rb_define_method(klass, "root=", set_root, 1);
  rb_define_method(klass, "serialize", serialize, 0);
}


/* public API */
VALUE Nokogiri_wrap_xml_document(xmlDocPtr doc)
{
  VALUE klass = rb_eval_string("Nokogiri::XML::Document");
  return Data_Wrap_Struct(klass, 0, dealloc, doc) ;
}
