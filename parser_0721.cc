//  PARSER.CC  Ver. 1.0
//  Program to extract Nodal equations from Spice Netlist.  Ed Chan

#include "parser.h"

double stripString( char *stringIn );

main( int argc, char *argv[] ) {
  ifstream inFile;
  ofstream outFile;
  NodeHead nodeList;
  CompHead compList;
  ModelHead modelList;

  // Buffers used in parsing:
  char inName[NameLength], outName[NameLength], buf[BufLength], 
    buf1[BufLength], buf2[BufLength], buf3[BufLength], nameBuf[NameLength],
    *bufPtr, *charPtr1, *charPtr2;
  int intBuf1, intBuf2, intBuf3, intBuf4, datum=NA, eqNum=NA;
  double douBuf1, douBuf2, douBuf3, douBuf4;
  CompType typeBuf;
  Component *compPtr, *compPtr1, *compPtr2;
  Node *nodePtr, *nodePtr1, *nodePtr2;
  Model* modelPtr;
  TranType TtypeBuf;
  EquaType eqType = Modified;

  strcpy( inName, "NOTHING" );
  strcpy( outName, "NOTHING" );

  //  processing command line
  if (argc == 1){
    cerr << "USAGE:  parser -f FILENAME <-d DATUM> <-e EQUATION-TYPE> <-o OUTPUT_FILE>" << endl;
    exit(0);
  }
  else {
    for( int c=1; c < argc; c++ ){
      if( (c==1) && isalpha( *argv[c] ) )
	strcpy( inName, argv[c] );
      else if( !strcmp( argv[c], "-f" ) ){
	if( (c+1) > argc ){
	  cerr << "USAGE:  parser -f FILENAME <-d DATUM> <-e EQUATION-TYPE> <-o OUTPUT_FILE>" << endl;
	  exit(0);
	}
	strcpy( inName, argv[c+1] );
      }
      else if( !strcmp( argv[c], "-d" ) ){
	if( (c+1) > argc ){
	  cerr << "USAGE:  parser -f FILENAME <-d DATUM> <-e EQUATION-TYPE> <-o OUTPUT_FILE>" << endl;
	  exit(0);
	}
	datum = atoi( argv[c+1] );
      }
      else if( !strcmp( argv[c], "-e" ) ){
	if( (c+1) > argc ){
	  cerr << "USAGE:  parser -f FILENAME <-d DATUM> <-e EQUATION-TYPE> <-o OUTPUT_FILE>" << endl;
	  exit(0);
	}
	eqNum = atoi( argv[c+1] );
      }
      else if( !strcmp( argv[c], "-o" ) ){
	if( (c+1) > argc ){
	  cerr << "USAGE:  parser -f FILENAME <-d DATUM> <-e EQUATION-TYPE> <-o OUTPUT_FILE>" << endl;
	  exit(0);
	}
	strcpy( outName, argv[c+1] );
      }
    }
  }  
  
  // process equation types:
  if ( eqNum != NA ){
    while( (eqNum != 1) && (eqNum != 2) ){
      cout << "Available Equations Types Are:" << endl
	   << " <1>  Nodal" << endl
	   << " <2>  Modified Nodal" << endl
	   << "Please enter your choice <1, 2>:" << endl;
      cin >> buf;
      eqNum = atoi( buf );
    }
    if( eqNum == 1 )
      eqType = Nodal;
    else if( eqNum == 2 )
      eqType = Modified;
  }
  
  // process input file name:
  if( !strcmp( inName, "NOTHING" ) ){
    cerr << "Please enter the input Spice Netlist: <QUIT to exit>" << endl;
    cin >> inName;
    if( !strcmp( inName, "QUIT" ) ){
      cerr << "Program Exited Abnormally!" << endl;
      exit(0);
    }
  }
  inFile.open( inName, ios::in );
  while( !inFile ){
    cerr << inName << " is an invalid input file." << endl
	 << "Please enter the input Spice Netlist: <QUIT to exit>" << endl;
    cin >> inName;
    if( !strcmp( inName, "QUIT" ) ){
      cerr << "Program Exited Abnormally!" << endl;
      exit(0);
    }
    inFile.open( inName, ios::in );
  }
  
  // process output file
  if( !strcmp( outName, "NOTHING" ) ){
    strcpy( outName, inName );
    strtok( outName, "." );
    strcat( outName, ".Pout" );
  }
  outFile.open( outName, ios::out );
  cout << "Output saved to file: " << outName << endl;


  // parsing of netlist to create linked list of models (remember to reset the fstream)
  inFile.getline(buf, BufLength);       // first line of netlist is discarded
  inFile.getline(buf, BufLength);
  while( inFile.good() ){
    if( (buf == NULL ) || (*buf == '\0') ){
      inFile.getline(buf, BufLength);
      continue;
    }
    strcpy( buf1, buf );
    if( !strcmp( strtok( buf1, " " ), ".model" ) ){
      strcpy( buf2, strtok( NULL, " " ) );
      charPtr1 = strtok( NULL, " " );
      if( !strcmp( charPtr1, "PNP" ) )
	TtypeBuf = PNP;
      else if( !strcmp( charPtr1, "NPN" ) )
	TtypeBuf = NPN;
      else if( !strcmp( charPtr1, "NMOS" ) )
	TtypeBuf = NMOS;
      else if( !strcmp( charPtr1, "PMOS" ) )
	TtypeBuf = PMOS;
    
      charPtr1 = strtok( NULL, " " );
      while ( charPtr1 != NULL ){
	strcpy( buf3, "" );
	if( (charPtr1[0] == 'I') && (charPtr1[1] == 'S') && (charPtr1[2] == '=') ){
      	  douBuf1 = stripString( charPtr1 );
	}
	if( (charPtr1[0] == 'B') && (charPtr1[1] == 'F') && (charPtr1[2] == '=') ){
	  douBuf2 = stripString( charPtr1 );
	}
	if( (charPtr1[0] == 'B') && (charPtr1[1] == 'R') && (charPtr1[2] == '=') ){
	  douBuf3 = stripString( charPtr1 );
	}
	if( (charPtr1[0] == 'T') && (charPtr1[1] == 'E') && (charPtr1[4] == '=') ){
	  douBuf4 = stripString( charPtr1 );
	}
	charPtr1 = strtok( NULL, " " );
      }
      modelPtr = new Model( buf2, TtypeBuf, douBuf1, douBuf2, douBuf3, douBuf4 );
      modelList.addModel( modelPtr );
    }
    inFile.getline(buf, BufLength);
  }
  inFile.close();
  inFile.open( inName, ios::in );


  //  starting of parsing by creating linked list of components
  inFile.getline(buf, BufLength);       // first line of netlist is discarded
  inFile.getline(buf, BufLength);
  while( inFile.good() ){
    if( (buf==NULL) || (*buf=='\0') ){
      inFile.getline(buf, BufLength);
      continue;
    }
    if( isalpha( *buf ) ){
      
      //  EDIT THIS SECTION IF NEW COMPONENTS ARE ADDED!!!
      //  we could do some rearranging in this section to catch each type in order.
      switch( *buf ) {
      case 'v':
      case 'V':
	typeBuf = VSource;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	douBuf1 = atof( strtok( NULL, " " ) );
	compPtr = new Component( typeBuf, douBuf1, NA,  intBuf1, intBuf2, NA, NA, NULL, nameBuf );
	compList.addComp( compPtr );
	break;
      case 'i':
      case 'I':
	typeBuf = ISource;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	douBuf1 = atof( strtok( NULL, " " ) );
	compPtr = new Component( typeBuf, douBuf1, NA,  intBuf1, intBuf2, NA, NA, NULL, nameBuf );
	compList.addComp( compPtr );
	break;
      case 'q':
      case 'Q':
	typeBuf = BJT;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	intBuf3 = atoi( strtok( NULL, " " ) );
	compPtr = new Component( typeBuf, NA, NA, intBuf1, intBuf2, intBuf3, NA, 
				 modelList.getModel( strtok( NULL, " " ) ), nameBuf );
	compList.addComp( compPtr );
	break;
      case 'm':
      case 'M':
	typeBuf = MOSFET;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	intBuf3 = atoi( strtok( NULL, " " ) );
	intBuf4 = atoi( strtok( NULL, " " ) );	
	compPtr = new Component( typeBuf, NA, NA, intBuf1, intBuf2, intBuf3, intBuf4, 
				 modelList.getModel( strtok( NULL, " " ) ), nameBuf );
	compList.addComp( compPtr );
	break;
      case 'r':
      case 'R':
	typeBuf = Resistor;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	douBuf1 = atof( strtok( NULL, " " ) );
	compPtr = new Component( typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf );
	compList.addComp( compPtr );
	break;
      case 'd':
      case 'D':
	typeBuf = Diode;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	charPtr1 = strtok( NULL, " " );
	while( charPtr1 != NULL ){
	  if( (charPtr1[0] == 'I') && (charPtr1[1] == 'S') && (charPtr1[2] == '=') ){
	    douBuf1 = stripString( charPtr1 );
	  }	    
	  if( (charPtr1[0] == 'T') && (charPtr1[1] == 'E') && (charPtr1[4] == '=') ){
	    douBuf2 = stripString( charPtr1 );
	  }
	  charPtr1 = strtok( NULL, " " );
	}
	compPtr = new Component( typeBuf, douBuf1, douBuf2, intBuf1, intBuf2, NA, NA, NULL, nameBuf );
	compList.addComp( compPtr );
	break;
      case 'c':
      case 'C':
	typeBuf = Capacitor;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	douBuf1 = atof( strtok( NULL, " " ) );
	compPtr = new Component( typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
	compList.addComp( compPtr );
	break;
      case 'l':
      case 'L':
	typeBuf = Inductor;
	strcpy( nameBuf, strtok( buf, " " ) );
	intBuf1 = atoi( strtok( NULL, " " ) );
	intBuf2 = atoi( strtok( NULL, " " ) );
	douBuf1 = atof( strtok( NULL, " " ) );
	compPtr = new Component( typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
	compList.addComp( compPtr );
	break;
      };
    }
    inFile.getline(buf, BufLength);
  }


//  Now the components are created and it is time to set up the list of nodes.
//  we should actually use second connector of first Source as the first Node (Datum)
  compPtr1 = compList.getComp(0);
  while( compPtr1 != NULL ){
	for ( int b=0; b < 3; b++ ){
	  if( (!compPtr1->isCon( b )) && ( compPtr1->getConVal( b ) != NA ) ){
	intBuf1 = compPtr1->getConVal( b );
	nodePtr1 = nodeList.addNode();
	nodePtr1->setNameNum( intBuf1 );
	compPtr1->connect( b, nodePtr1 );
	nodePtr1->connect( b, compPtr1 );
	
	// now search and connect all other appropriate connectors to this node.
	// error checking should be added to prevent duplicated, or skipped connectors.
	compPtr2 = compPtr1->getNext();
	while( compPtr2 != NULL ){
	  for( int c=0; c < 3; c++ ){
	    if( compPtr2->getConVal( c ) == intBuf1  ){
	      compPtr2->connect( c, nodePtr1 );
	      nodePtr1->connect( c, compPtr2 );
	      break;
	    }
	  }  
	  compPtr2 = compPtr2->getNext();
	}
      }
    }
    compPtr1 = compPtr1->getNext();
  }

//  At this point, we are done creating a representation of the circuit in memory
//  now, we need to call each node to create and output its nodal equation.
//  Each node will call the components attached for the individual contributions to the
//  nodal equation.

  // verify that input datum is valid
  Boolean check = FALSE;
  if( datum != NA ){
    nodePtr = nodeList.getNode(0);
    while( nodePtr != NULL ){
      if( nodePtr->getNameNum() == datum )
	check = TRUE;
      nodePtr = nodePtr->getNext();
    }
    if( check == FALSE ){
      cerr << "Datum value invalid!" << endl
	   << "PROGRAM EXITED ABNORMALLY!" << endl;
      exit(0);
    }
  }
  
// Loop to find lastnode
  nodePtr = nodeList.getNode(0);
  int lastnode = nodePtr->getNameNum();
  while( nodePtr != NULL ){
    lastnode = ( nodePtr->getNameNum() > lastnode )? nodePtr->getNameNum(): lastnode;
    nodePtr = nodePtr->getNext();
  }

//  Loop to find the datum
  if( datum == NA ){
    nodePtr = nodeList.getNode(0);
    nodePtr1 = nodePtr->getNext();
    while( nodePtr1 != NULL ){
      if( nodePtr1->getCount() > nodePtr->getCount() )
	nodePtr = nodePtr1;
      nodePtr1 = nodePtr1->getNext();
    }
    datum = nodePtr->getNameNum();
  }


  // output circuit information
  outFile << "%Parser V1.0" << endl;
  outFile << "%Input Spice Deck:  " << inName << endl;
  outFile << "%Equation Type:     ";
  if( eqType == Nodal ) 
    outFile << "NODAL" << endl;
  else if( eqType == Modified ) 
    outFile << "MODIFIED NODAL" << endl;
  outFile << "%Datum Node:        " << datum << endl;


  // create value table
  // 遍历所有节点并打印信息
  outFile << endl
	  << "%*****************************************************************************" << endl;
  outFile << "Title:                     *测试tran的电路:" << endl;
  nodePtr = nodeList.getNode(0);
  while (nodePtr != NULL) {
	  printNodeInfo(nodePtr, outFile);
	  nodePtr = nodePtr->getNext();
  }
  outFile << endl 
	  << "%*****************************************************************************" << endl;


  // go down the nodal list and have components announce themselves
  outFile << endl << "%                      Circuit Equations: " << endl;
  nodePtr = nodeList.getNode(0);
  while( nodePtr != NULL ){
    if( nodePtr->getNameNum() != datum ){
      nodePtr->printComps( outFile, datum, eqType );
    }
    nodePtr = nodePtr->getNext();
  }
  
  // go down the component list and give equations for all sources
  compPtr = compList.getComp(0);
  while( compPtr != NULL ){
    compPtr->specialPrint( outFile, datum, lastnode );
    compPtr = compPtr->getNext();
  }
  
  // print jacobians
outFile << endl 
	<< "%*****************************************************************************" << endl;
  outFile << endl << "%                      Jacobians: " << endl;
  nodePtr1 = nodeList.getNode(0);
  while( nodePtr1 != NULL ){
    if( nodePtr1->getNameNum() != datum ){
      nodePtr2 = nodeList.getNode(0);
      while( nodePtr2 != NULL ){
	if( nodePtr2->getNameNum() != datum ){
	  nodePtr1->printJac( outFile, datum, eqType, nodePtr2 );
	}
	nodePtr2 = nodePtr2->getNext();
      }
    }
    nodePtr1 = nodePtr1->getNext();
  }
  
  // go down the component list and give equations for all sources
  compPtr = compList.getComp(0);
  while( compPtr != NULL ){
    nodePtr2 = nodeList.getNode(0);
    while( nodePtr2 != NULL ){
      if( nodePtr2->getNameNum() != datum ){
	compPtr->specialPrintJac( outFile, datum, nodePtr2, lastnode );
      }
      nodePtr2 = nodePtr2->getNext();
    }
    compPtr = compPtr->getNext();
  }



  cout << endl << "That's all folks!" << endl;    
  return 0;
}




double stripString( char *stringIn ){
  char buf[BufLength], buf2[BufLength];
  int a, b;
  strcpy( buf, stringIn );
  for( a=0; buf[a] != '='; a++ ){};
  a++;
  for( b=0; buf[a] != '\0'; b++, a++ )
    buf2[b] = buf[a];
  buf2[b] = '\0';
  return atof( buf2 );
};













