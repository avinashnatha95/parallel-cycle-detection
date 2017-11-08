#include <stdio.h>
#include <stdlib.h>
#include <metis.h>
#include <string.h>

char clr[100];				//Colour of each vertex
int reach[100][100];		//Reachability information of every vertex
int trans[100][100];		//All the trans arcs in the graph
int expgh[100][100];		//The final express graph created
int entv[100],exitv[100];	//All the entrance and exit vertices
int terminate = 0;			//Flag to determine that cycle is detected

idx_t nParts = 2; 			//No .of parts graph is divided

FILE *bp;

//Create a file which is used by Graphviz tool to show the graph
void graphwriter(int g[100][100],int nv,int part[100],char* filename)
{	
	char photoname[30];
	char filename2[30];
	char spt[2] = ".";
	char *token;

	strcpy(filename2,filename);
	token = strtok(filename2,spt);
	sprintf(photoname,"%s.png",token);
	fprintf(bp, "dot -Tpng %s > %s\n",filename, photoname); //Command appended in the script file to create the graphs.
	
	FILE *fp;
	int i,j;
	char *color;

	fp = fopen(filename,"w");

	fprintf(fp,"digraph finalgraph {\n");

	if(nParts != 1)
	{
		for(j=0;j<nParts;j++)
		{
			fprintf(fp,"subgraph cluster_%d {\n",j);
			fprintf(fp,"label=\"Part %d\";\n",j+1);
			
			for(i=1;i<=nv;i++)	
			{
				if(part[i-1] == j)
				{
					if(clr[i] == 'w')
					{
						color = "white";
					}
					else if(clr[i] == 'b')
					{
						color = "grey";
					}
					else if(clr[i] == 'r')
					{
						color = "red";
					}
					else if(clr[i] == 'g')
					{
						color = "green";
					}

					fprintf(fp,"%d [style=filled,fillcolor=%s];\n",i,color);
				}
			}
			fprintf(fp, "}\n" );
		}
	}
	else
	{
		for(i=1;i<=nv;i++)	
		{
			if(clr[i] == 'w')
			{
				color = "white";
			}
			else if(clr[i] == 'b')
			{
				color = "grey";
			}
			else if(clr[i] == 'r')
			{
				color = "red";
			}
			else if(clr[i] == 'g')
			{
				color = "green";
			}

			fprintf(fp,"%d [style=filled,fillcolor=%s];\n",i,color);\
		}
	}

	for(i=1;i<=nv;i++)
	{
		for(j=1;j<=g[i][0];j++)
		{
			fprintf(fp, "%d -> %d;\n",i,g[i][j]);
		}
	}

	fprintf(fp, "}\n");
}

//The first phase of algorithm which does the cycle detection in every part and finds reachability information
void visit(int v,int g[100][100],int ver[100][100],int part[100],int npc,int nv)
{
	int i,j,k;

	clr[v] = 'r';

	for(i=1;i<=g[v][0];i++)
	{
		if(part[v-1] == part[g[v][i]-1])
		{
			//if the arc is a local-arc
			if(clr[g[v][i]] == 'w')
			{
				visit(g[v][i],g,ver,part,npc,nv);

				for(j=1;j<=reach[g[v][i]][0];j++)
				{
					int found = 0;

					for(k=1;k<=reach[v][0];k++)
					{
						if(reach[v][k] == reach[g[v][i]][j])
						{
							found = 1;
							break;
						}
					}

					if(found == 1)
					{
						continue;
					}
					else
					{
						reach[v][0]++;
						reach[v][reach[v][0]] = reach[g[v][i]][j];
					}
				}
			}
			else if(clr[g[v][i]] == 'r')
			{
				printf("Cycle has been detected.\n");
				graphwriter(g,nv,part,"graph_photos/graphoutput.gv");
				exit(-1);
			}
			else if(clr[g[v][i]] == 'b')
			{
				for(j=1;j<=reach[g[v][i]][0];j++)
				{
					int found = 0;

					for(k=1;k<=reach[v][0];k++)
					{
						if(reach[v][k] == reach[g[v][i]][j])
						{
							found = 1;
							break;
						}
					}

					if(found == 1)
					{
						continue;
					}
					else
					{
						reach[v][0]++;
						reach[v][reach[v][0]] = reach[g[v][i]][j];
					}
				}
			}
		}
		else
		{
			//if the arc is a trans-arc
			int found = 0;

			for(k=1;k<=reach[v][0];k++)
			{
				if(reach[v][k] == v)
				{
					found = 1;
					break;
				}
			}

			if(found == 0)
			{
				reach[v][0]++;
				reach[v][reach[v][0]] = v;
			}

			found = 0;

			for(k=1;k<=trans[v][0];k++)
			{
				if(trans[v][k] == g[v][i])
				{
					found = 1;
					break;
				}
			}

			if(found == 0)
			{
				trans[v][0]++;
				trans[v][trans[v][0]] = g[v][i];
			}
		}
	}

	if(reach[v][0] == 0)
		clr[v] = 'g';
	else
		clr[v] = 'b';

	return;
}

int main()
{
	int i,j,g[100][100],t,ne,a,b,ug[100][100],k,nv,l,m,fct;
	//g[][] 	-	The main input graph
	//ug[][] 	-	The main graph converted and stored as undirected graph
	
	char fname[20] = "graphmid";
	char lfn[30];
	fct = 0;

	bp = fopen("graph_photos/script.sh","w");

	printf("Enter number of vertices:\n");
	scanf("%d",&nv);
	printf("Enter number of edges:\n");
	scanf("%d",&ne);
	//nv - Total number of vertices in the graph
	//ne - Total number of edges in the graph

	entv[0] = 0;
	exitv[0] = 0;

	for (i = 0; i <= nv; i++)
	{
		expgh[i][0] = 0;
		g[i][0] = 0;
		ug[i][0] = 0;
		reach[i][0] = 0;
		clr[i] = 'w';
	}

	//Scaninng the input
	printf("Enter the edges:\n");
	for(i=0;i<ne;i++)
	{
		scanf("%d %d",&a,&b);

		g[a][0]++;
		g[a][g[a][0]] = b;
	}

	//Converting the input graph to undirected graph so that it can be used by MeTiS for partitioning
	for(i=1;i<=nv;i++)
	{
		for(j=1;j<=g[i][0];j++)
		{
			int breaker = 0;

			a = i;
			b = g[i][j];

			for(k=1;k<ug[a][0];k++)
			{
				if(ug[a][k] == b)
				{
					breaker = 1;
					break;
				}
			}

			if(breaker == 0)
			{
				ug[a][0]++;
				ug[a][ug[a][0]] = b;

				ug[b][0]++;
				ug[b][ug[b][0]] = a;
			}
		}
	}

	//Graph partition using MeTiS
	idx_t nVertices = nv;
    idx_t nEdges    = ne;
    idx_t nWeights  = 1;

	idx_t objval;
	idx_t part[nVertices];

	idx_t xadj[nVertices+1];
	idx_t adjncy[2*nEdges];

	xadj[0] = 0;

	for(i=1;i<=nv;i++)
	{
		xadj[i] = xadj[i-1] + ug[i][0];
	}

	for(i=0;i<nVertices;i++)
	{
		for(j=xadj[i];j<xadj[i+1];j++)
		{
			adjncy[j] = ug[i+1][j-xadj[i]+1]-1;
		}
	}

	//The function which does the graph partition in MeTiS 
	int ret = METIS_PartGraphRecursive(&nVertices,&nWeights, xadj, adjncy,NULL, NULL, NULL, &nParts, NULL,NULL, NULL, &objval, part); 
	//int ret = METIS_PartGraphKway(&nVertices,&nWeights, xadj, adjncy,NULL, NULL, NULL, &nParts, NULL,NULL, NULL, &objval, part);

	graphwriter(g,nv,part,"graph_photos/graphinput.gv");

	for(i=0;i<nVertices;i++)
		printf("%d belongs to %d part\n",i+1,part[i]);
	printf("\n");

	int ver[100][100]; //The set of vertices grouped by partition they belong

	for (i = 0; i < nParts; ++i)
	{
		ver[i][0] = 0;
	}

	for(i=1;i<=nVertices;i++)
	{
		ver[part[i-1]][0]++;
		ver[part[i-1]][ver[part[i-1]][0]] = i;
	}

	int npc;
	
	for(i=1;i<=nv;i++)
	{
		trans[i][0] = 0;
	}

	//calling the first phase of the algorithm for every part of the graph
	for(npc=0;npc<nParts;npc++)
	{
		for(i=0;i<ver[npc][0];i++)
		{
			if(clr[ver[npc][i]] == 'w')
				visit(ver[npc][i],g,ver,part,npc,nv);
		}
	}
	if(nParts == 1) //If number of partitions is 1, the above function visit doesn't get called
	{
		for(i=0;i<ver[npc][0];i++)
		{
			if(clr[ver[npc][i]] == 'w')
				visit(ver[npc][i],g,ver,part,npc,nv);
		}
	}

	//Printing reachability information of each vertex
	for(i=1;i<=nVertices;i++)
	{
		printf("Rv of %d ->",i);
		for(j=1;j<=reach[i][0];j++)
			printf("%d ",reach[i][j]);
		printf("\n");
	}
	printf("\n");

	//Finding all the entrance and exit vertices
	for(i=1;i<=nVertices;i++)
	{
		if(trans[i][0] != 0)
		{
			exitv[0]++;
			exitv[exitv[0]] = i;
		}
		for(j=1;j<=trans[i][0];j++)
		{
			int breaker = 0;

			for(k=1;k<=entv[0];k++)
			{
				if(entv[k] == trans[i][j])
				{
					breaker = 1;
					break;
				}
			}

			if(breaker == 1)
			{
				continue;
			}
			else
			{
				entv[0]++;
				entv[entv[0]] = trans[i][j];
			}
		}
	}

	printf("Entrance vertices are ");
	for(i=1;i<=entv[0];i++)
	{
		printf("%d ",entv[i]);
	}
	printf("\n");

	printf("Exit vertices are ");
	for(i=1;i<=exitv[0];i++)
	{
		printf("%d ",exitv[i]);
	}
	printf("\n\n");

	//adding express arcs to the express graph
	for(i=1;i<=entv[0];i++)
	{
		for(j=1;j<=reach[entv[i]][0];j++)
		{
			expgh[entv[i]][0]++;
			expgh[entv[i]][expgh[entv[i]][0]] = reach[entv[i]][j];
		}
	}

	fct++;
	sprintf(lfn,"graph_photos/%s%d.gv",fname,fct);
	graphwriter(expgh,nv,part,lfn);

	int cyclefnd = 0;	//Flag to check if cycle is found in the express graph 

	//adding trans arcs to express graph.
	for(i=1;i<=nVertices;i++)
	{
		for(j=1;j<=trans[i][0];j++)
		{
			a = i;
			b = trans[i][j];

			for(k=1;k<=expgh[b][0];k++)
			{
				if(expgh[b][k] == a)
				{
					cyclefnd = 1;
					expgh[a][0]++;
					expgh[a][expgh[a][0]] = b;
				}
			}

			if(cyclefnd == 1)
			{
				break;
			}
			else
			{
				expgh[i][0]++;
				expgh[i][expgh[i][0]] = trans[i][j];
				fct++;
				sprintf(lfn,"graph_photos/%s%d.gv",fname,fct);
				graphwriter(expgh,nv,part,lfn);
			}

			int succ[100],pred[100];
			succ[0] = 0;
			pred[0] = 0;

			for(k=1;k<=reach[b][0];k++)
			{
				succ[0]++;
				succ[succ[0]] = reach[b][k];
			}

			for(k=1;k<=entv[0];k++)
			{
				for(l=1;l<=reach[entv[k]][0];l++)
				{
					if (reach[entv[k]][l] == a)
					{
						pred[0]++;
						pred[pred[0]] = entv[k];
					}
				}
			}

			for(k=1;k<=pred[0];k++)
			{
				for(l=1;l<=succ[0];l++)
				{
					a = pred[k];
					b = succ[l];

					for(m=1;m<=expgh[b][0];m++)
					{
						if(expgh[b][m] == a)
						{
							cyclefnd = 1;
							printf("loop edge - %d %d\n",a,b);
							expgh[a][0]++;
							expgh[a][expgh[a][0]] = b;
						}
					}

					if(cyclefnd == 0)
					{
						expgh[a][0]++;
						expgh[a][expgh[a][0]] = b;
						fct++;
						sprintf(lfn,"graph_photos/%s%d.gv",fname,fct);
						graphwriter(expgh,nv,part,lfn);
					}
					else
					{
						break;
					}
				}
				if(cyclefnd == 1)
					break;
			}
			if(cyclefnd == 1)
				break;
		}
		if(cyclefnd == 1)
			break;
	}

	//Calling the function to output the express graph
	graphwriter(expgh,nv,part,"graph_photos/graphoutput.gv");

	if(cyclefnd == 0)
	{
		printf("No cycle has been detected.\n");
	}
	else
	{
		printf("Cycle has been detected.\n");
	}
	
	return 0;
}