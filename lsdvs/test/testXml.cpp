#if 0

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "mxml.h"
#include "gb2312.h"
#include "debug.h"
#include "test.h"
#include "Malloc.h"

#define CHINESE_STR_TEST "sven"
#define XML_HEAD_TEST "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
static char cmd_buf[1024];

void TestXmlMakeTermRegisterPack()
{    
	int ret;
	int len;
	mxml_node_t *tree,*node,*root;
	char *putf;

	FILE *fd = fopen( "./xml.txt", "wr" );
	FiPrint2( "fd -------------------- (%X)\r\n", fd );
    
	tree = mxmlLoadString(NULL, XML_HEAD_TEST, MXML_NO_CALLBACK);    
	root = mxmlNewElement( tree, "TermRegister" );
	mxmlElementSetAttr(root, "DeviceID", "01000010400000000000");//pTermRegister->attr.device_id);
	mxmlElementSetAttr(root, "DeviceIP", "192.168.18.109");
	ret = Gb2312ToUtf8( (char *)CHINESE_STR_TEST, strlen(CHINESE_STR_TEST), &putf );
	FiPrint2( "ret(%d) = Gb2312ToUtf8!\r\n", ret );
	mxmlElementSetAttr(root, "DeviceLinkType", putf );
	Free( putf );
	mxmlElementSetAttr(root, "DeviceMaxConnect","2");
	node = mxmlNewElement( root, "VideoPort" );
	mxmlNewTextf(node,0,"%d",4006);
    
	node = mxmlNewElement(root,"AudioPort");
	mxmlNewTextf(node,0,"%d",4007);

	node = mxmlNewElement(root,"MsgPort");
	mxmlNewTextf(node,0,"%d",4008);

	node = mxmlNewElement(root,"Version");
	mxmlNewTextf(node,0,"%d",0);
    
	node = mxmlNewElement(root,"SchemeSupport");
	mxmlNewTextf(node,0,"%d",1);
	node = mxmlNewElement(root,"PtzLockRet");
	mxmlNewTextf(node,0,"%d",0);    
    
	node = mxmlNewElement(root,"Protocol");
	mxmlNewTextf(node,0,"%s", "ST_PLAY");
    
	node = mxmlNewElement(root,"Support");
	mxmlElementSetAttr(node,"NAT","true");
	mxmlElementSetAttr(node,"Scheme","true");
	mxmlElementSetAttr(node,"PtzLockRet","true");
	mxmlElementSetAttr(node,"Video2","true");    

	node = mxmlFindElement( tree, tree, "TermRegister", NULL, NULL, MXML_DESCEND );
	SVPrint( "node(0x%X)!\r\n", node );
	mxmlElementSetAttr( node, "InsertTest", "InsertAttr" );     

	mxmlSaveFile( tree, fd, MXML_NO_CALLBACK );
	fclose( fd );
	sync();
    
	mxmlSaveString(tree,cmd_buf,1024,MXML_NO_CALLBACK);
	mxmlDelete(tree);
	len = strlen(cmd_buf);
    //len = whitespace_cb(cmd_buf,len);

	SVPrint("xml len: %d, xml content: \r\n%s\r\n",len,cmd_buf);    
}

void TestXmlMakeTermRegisterParse()
{
	mxml_node_t *tree,*node,*root, *child;
	char *buf = cmd_buf;    
    
	tree = mxmlLoadString(NULL,buf,MXML_NO_CALLBACK);    
	root = tree;

    //node = mxmlFindElement( tree, tree,"TermRegister", NULL,NULL, MXML_DESCEND);
	node = mxmlFindElement( tree, tree,"VideoPort", NULL,NULL, MXML_DESCEND);
	child = mxmlGetFirstChild( node );
	FiPrint2( "mxmlFindElement( VideoPort )=0x%X, node(0x%x)!\r\n\r\n", child, node );    
	FiPrint2( "node->child=0x%X!\r\n\r\n", node->child );
	FiPrint2( "VideoPort = %s!\r\n\r\n", node->child->value.text.string );        
	FiPrint2( "VideoPort = %s!\r\n\r\n", child->value.text.string );    
	FiPrint2( "VideoPort = %d!\r\n\r\n", node->child->value.opaque );

	FiPrint2( "tree->value.text.string = 0x%X!\r\n\r\n", node->value.text.string );
	FiPrint2( "node->child->value.text.string = 0x%X!\r\n\r\n", node->child->value.text.string );

	mxmlDelete(tree);
}

void TestXmlMakeTermRegisterParseLoadHead()
{
	mxml_node_t *top, *tree,*node,*root, *child;
	char *buf = cmd_buf;    
	char ql_buf[4096];

	top = mxmlLoadString( NULL, XML_HEAD_TEST, MXML_NO_CALLBACK );    
	tree = mxmlLoadString( top, buf, MXML_NO_CALLBACK );    

	mxmlSaveString( top, ql_buf, sizeof(ql_buf), MXML_NO_CALLBACK );
    
	SVPrint("ql_buf xml content: \r\n%s\r\n", ql_buf);    
	root = tree;

    //node = mxmlFindElement( tree, tree,"TermRegister", NULL,NULL, MXML_DESCEND);
	node = mxmlFindElement( tree, tree,"VideoPort", NULL,NULL, MXML_DESCEND);
	child = mxmlGetFirstChild( node );
	FiPrint2( "mxmlFindElement( VideoPort )=0x%X!\r\n\r\n", child );    
	FiPrint2( "node->child=0x%X!\r\n\r\n", node->child );
	FiPrint2( "VideoPort = %s!\r\n\r\n", node->child->value.text.string );    
	FiPrint2( "VideoPort = %d!\r\n\r\n", node->child->value.opaque );

	FiPrint2( "tree->value.text.string = 0x%X!\r\n\r\n", node->value.text.string );
	FiPrint2( "node->child->value.text.string = 0x%X!\r\n\r\n", node->child->value.text.string );

	mxmlDelete(tree);
}

#endif 

