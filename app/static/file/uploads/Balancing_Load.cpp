#include <algorithm>
#include <bitset>
#include <complex>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <new>
#include <numeric>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <typeinfo>
#include <utility>
#include <valarray>
#include <vector>

using namespace std;

const double INTMAX = 1000000;
const double load_weight = 1;
double penalty_weight;
double load_threshold;
const int numOfVar = 6;
double n,E[10][1001],lv[1001], x_init[1001];
double total_load[numOfVar] = {0,0,0,0,0,0};
string id = "";

double power(double al, int expo) {
	if (expo > 50) return INTMAX;
	if (expo == 0) return 1;
	if (expo == 1) return al;
	double cmp = power(al, expo/2);
	if (expo % 2 == 0) return min(INTMAX,cmp*cmp); else return min(INTMAX,cmp*cmp*al);
}

struct Gene {
	int data;
	Gene* next;
	Gene(int x){
		data = x;
		next = NULL;
	}
	~Gene() {
		
	}
};

struct Chromosome {
	public:
	int length = 0;
	double fitness = INTMAX;
	double electric_value[numOfVar] = {INTMAX,INTMAX,INTMAX,INTMAX,INTMAX,INTMAX};
	Gene* firstGene;
	Chromosome* next;
	Chromosome() {
		next = NULL;
		length = 0;
	}
	~Chromosome() {
		Gene* g = firstGene;
		while (g != NULL) {
			Gene* gg = g;
			g = g->next;
			gg->next = NULL;
			delete gg;
		}
	}
	void addGene(Gene* gene) {
		if (length == 0) firstGene = gene; else {
			Gene* ptr = firstGene;
			while (ptr->next != NULL) ptr = ptr->next;
			ptr->next = gene;

		}
		++length;
	}

	bool isFull() {
		return length == n;
	}

	bool evaluateFitness() {
		if (!isFull()) {
			cout<<"Error!!!Not enough genes for evaluate!!!";
			return false;
		}
		Gene* ptr = firstGene;
		fitness = 0;
		for (int var = 0; var < numOfVar; ++var) {
			electric_value[var] = 0;
		}
		int x[length+1];
		int i=0;
		while (ptr != NULL) {
			x[i] = ptr->data;
			++i;
			ptr = ptr->next;
		}
		int load_A[numOfVar] = {0,0,0,0,0,0}, load_B[numOfVar] = {0,0,0,0,0,0}, load_C[numOfVar] = {0,0,0,0,0,0};
		for (int var = 0; var < numOfVar; ++var) {
			for (int i=0; i<n; ++i) {
				if (x[i] == 0) {
				   load_A[var] += E[var][i];
				} else if (x[i] == 1) {
				   load_B[var] += E[var][i];
				} else {
				   load_C[var] += E[var][i];
				}
			}
		}
		for (int var = 0; var < numOfVar; ++var) {
			electric_value[var] = sqrt(((load_A[var] - load_B[var])*(load_A[var] - load_B[var]) + (load_C[var] - load_B[var])*(load_C[var] - load_B[var]) + (load_A[var] - load_C[var])*(load_A[var] - load_C[var]))/2);
			fitness = fitness + sqrt(((load_A[var] - load_B[var])*(load_A[var] - load_B[var]) + (load_C[var] - load_B[var])*(load_C[var] - load_B[var]) + (load_A[var] - load_C[var])*(load_A[var] - load_C[var]))/2);
		}
		fitness = fitness*load_weight;
		int penalty_point = 0;
		for (int i = 0; i < n; ++i) {
			if (x[i] != x_init[i]) {
			   ++penalty_point;
			}
		}
		fitness += penalty_weight*penalty_point;
		return true;
	}

	Chromosome* local_optimal(int pos, int lv) {
		if (!isFull()) {
			cout<<"Error!!!Not enough genes for mix!!";
			return NULL;
		}
		Chromosome* ch = new Chromosome();
		Gene* g = this->firstGene;
		int idx = 0;
		while (g != NULL) {
			if (idx == pos) {
				ch->addGene(new Gene(lv));
			}
				else {
				ch->addGene(new Gene(g -> data));
			}
			g = g->next;
			++idx;
		}
		return ch;
	}

	Chromosome* mix(Chromosome* pa2) {
		if (!isFull() || !pa2->isFull()) {
			cout<<"Error!!!Not enough genes for mix!!";
			return NULL;
		}

		Chromosome* pa1=this,*child = new Chromosome();
		int splitP1 = rand()%(length/2-1)+1;
		int splitP2 = splitP1+rand()%(length/2-1)+1;
		Gene* ptr1 = pa1->firstGene;
		Gene* ptr2 = pa2->firstGene;
		int i=0;
		while (ptr1!=NULL && ptr2!=NULL) {
			if (splitP1<=i && i<=splitP2) {
				child->addGene(new Gene(ptr1->data));
			}
				else {
				child->addGene(new Gene(ptr2->data));
			}
			++i;
			ptr1 = ptr1->next;
			ptr2 = ptr2->next;
		}
		return child;
	}

	void write() {
		cout << "The fitness of the solution is : " << fitness << endl;
		Gene* ptr = firstGene;
		while (ptr != NULL) {
			cout<<ptr->data<<' ';
			ptr = ptr->next;
		}
		cout << endl;
	}

	void write_diff() {
		Gene* ptr = firstGene;
		int countt = 0, i = 0;
		while (ptr != NULL) {
			countt += (ptr->data != x_init[i]);
			++ i;
			ptr = ptr->next;
		}
		cout << "The number of modification is : " << countt <<endl;
	}

	void write_load() {
		Gene* ptr = firstGene;
		double load_A[numOfVar] = {0,0,0,0,0,0}, load_B[numOfVar] = {0,0,0,0,0,0}, load_C[numOfVar] = {0,0,0,0,0,0};
		int i = 0;
		while (ptr != NULL) {
			for (int var = 0; var < numOfVar; ++var) {
				if (ptr->data == 0) {
				   load_A[var] += E[var][i];
				} else if (ptr->data == 1) {
				   load_B[var] += E[var][i];
				} else {
				   load_C[var] += E[var][i];
				}
			}
			++i;
			ptr = ptr->next;
		}
		for (int var = 0; var < numOfVar; ++var) {
			cout << "Load A of Parameter " << var + 1 <<" : "<<load_A[var]<<endl;
			cout << "Load B of Parameter " << var + 1 <<" : "<<load_B[var]<<endl;
			cout << "Load C of Parameter " << var + 1 <<" : "<<load_C[var]<<endl;
		}
	}
};

struct Population {
	int length = 0;
	double delta = 0;
	Chromosome* firstChromo = NULL;
	Population() {
		length = 0;
		firstChromo = NULL;
	}

	bool checkCoincident(Chromosome* chromo) {
		bool check = false;
		if (length == 0) return check;
		Chromosome *ptr = firstChromo;
		while (ptr!=NULL) {
			if (abs(ptr->fitness - chromo->fitness) <= delta) {
				check = true;
				break;
			}
			ptr = ptr->next;
		}
		//if (check) cout<<"haha";
		return check;
	}

	void addChromosome(Chromosome* chromo) {
		if (!chromo->evaluateFitness()) {
			delete chromo;
			cout << "Cannot add chromosome";
			return;
		};
		if (checkCoincident(chromo)) {
			delete chromo;
			//cout << "Chromosome has already existed in the population";
			return;
		}
		
		if (length == 0) firstChromo = chromo; else {
			Chromosome* ptr = firstChromo;
			if (ptr->fitness > chromo->fitness) {
				chromo->next = ptr;
				firstChromo = chromo;
			}
			else {
				//firstChromo->next = chromo;
				Chromosome* ptr = firstChromo;
				while (ptr->next != NULL&&ptr->next->fitness < chromo->fitness) ptr = ptr->next;
				chromo->next = ptr->next;
				ptr->next = chromo;
			}
		}
		++length;
	}

	void terminateChromosome(int volume) {
		if (length<volume) return;
		Chromosome* ptr = firstChromo;
		for (int i=0;i<volume-1;++i) {
			ptr = ptr->next;
		}
		Chromosome* rm_chrono = ptr->next;
		ptr->next = NULL;
		while (rm_chrono != NULL) {
			//cout << rm_chrono << endl;
			Chromosome* rm_c = rm_chrono;
			rm_chrono = rm_chrono -> next;
			rm_c->next = NULL;
			delete rm_c;
		}
		length = volume;
	}

	double topFitness(int rankk) {
		Chromosome *ptr = firstChromo;
		for (int i=0;i<rankk;++i)
			ptr = ptr->next;
		return ptr->fitness;
	}

	double topElec(int rankk) {
		 Chromosome *ptr = firstChromo;
		 double min_Elect = INTMAX;
		
		 for (int i=0;i<rankk;++i) {
			 double total_electric_value = 0;
			 for (int var = 0; var < numOfVar; ++var) {
				 total_electric_value = total_electric_value + ptr->electric_value[var];
			 }
			min_Elect = min(total_electric_value, min_Elect);
			ptr = ptr->next;
		 }
		 return min_Elect;
   }

	void write() {
		Chromosome* ptr = firstChromo;
		while (ptr != NULL) {
			ptr->write();
			ptr = ptr->next;
		}
	}
};

Population P;

void Termination() {
	P.terminateChromosome(50);
}

void reducing_penalty(Chromosome* chromo) {
	Gene* g = chromo->firstGene;
	int load_A[numOfVar] = {0,0,0,0,0,0}, load_B[numOfVar] = {0,0,0,0,0,0}, load_C[numOfVar] = {0,0,0,0,0,0}, i = 0, diff = 0;
	 while (g != NULL) {
		 int coeff = g->data;
		 for (int var = 0; var < numOfVar; ++var) {
			 if (coeff == 0) {
				load_A[var] += E[var][i];
			 } else if (coeff == 1) {
				load_B[var] += E[var][i];
			 } else {
				load_C[var] += E[var][i];
			 }
		 }
		 diff += (coeff != x_init[i]);
		 ++i;
		 g = g->next;
	 }
	 g = chromo->firstGene;
	 i = 0;
	 while (g != NULL) {
		 int coeff = g->data;
		 double sum_min = INTMAX;
		 int saved = -1;

		 for (int var = 0; var < numOfVar; ++var) {
			 if (coeff == 0) {
				load_A[var] -= E[var][i];
			 } else if (coeff == 1) {
				load_B[var] -= E[var][i];
			 } else {
				load_C[var] -= E[var][i];
			 }
		 }
		 for (int k = 0; k <= 2; ++k) {
			 for (int var = 0; var < numOfVar; ++var) {
				 if (k == 0) {
					load_A[var] += E[var][i];
				 } else if (k == 1) {
					load_B[var] += E[var][i];
				 } else {
					load_C[var] += E[var][i];
				 }
			 }
			 double sum = 0;
			 for (int var = 0; var < numOfVar; ++var) {
				 sum = sum + sqrt(((load_A[var] - load_B[var])*(load_A[var] - load_B[var]) + (load_C[var] - load_B[var])*(load_C[var] - load_B[var]) + (load_A[var] - load_C[var])*(load_A[var] - load_C[var]))/2);
			 }
			if (k == x_init[i] && k != coeff) {
			   diff --;
			}
			sum = sum*load_weight + diff*penalty_weight;

			if (k == x_init[i] && k != coeff) {
			   diff ++;
			}
			if (k == x_init[i]) {
			   if (sum < sum_min) {
				  sum_min = sum;
				  //cout << "hello";
				  saved = k;
			   }
			}
			 for (int var = 0; var < numOfVar; ++var) {
				 if (k == 0) {
					load_A[var] -= E[var][i];
				 } else if (k == 1) {
					load_B[var] -= E[var][i];
				 } else {
					load_C[var] -= E[var][i];
				 }
			 }
		 }
		 for (int var = 0; var < numOfVar; ++var) {
			 if (coeff == 0) {
				load_A[var] += E[var][i];
			 } else if (coeff == 1) {
				load_B[var] += E[var][i];
			 } else {
				load_C[var] += E[var][i];
			 }
		 }

		 //cout << sum_min<<' '<<P.topFitness(30)<<' '<<saved<<endl;
		 if (saved != -1 && sum_min < P.topFitness(5)) {
			 //return;
			 //cout << "changed";
			 Chromosome * chro = chromo->local_optimal(i,saved);
			 P.addChromosome(chro);
			 //delete chro;
			 //cout << chro << endl;
		 }
		 g = g->next;
		 ++i;
	 }
}

void reducing_load(Chromosome* chromo) {
	 Gene* g = chromo->firstGene;
	int load_A[numOfVar] = {0,0,0,0,0,0}, load_B[numOfVar] = {0,0,0,0,0,0}, load_C[numOfVar] = {0,0,0,0,0,0}, i = 0;
	 while (g != NULL) {
		 int coeff = g->data;
		 for (int var = 0; var < numOfVar; ++var) {
			 if (coeff == 0) {
				load_A[var] += E[var][i];
			 } else if (coeff == 1) {
				load_B[var] += E[var][i];
			 } else {
				load_C[var] += E[var][i];
			 }
		 }
		 ++i;
		 g = g->next;
	 }
	 g = chromo->firstGene;
	 i = 0;
	 while (g != NULL) {
		 int coeff = g->data;
		 double sum_min = INTMAX;
		 int saved = -1;
		 for (int var = 0; var < numOfVar; ++var) {
			 if (coeff == 0) {
				load_A[var] -= E[var][i];
			 } else if (coeff == 1) {
				load_B[var] -= E[var][i];
			 } else {
				load_C[var] -= E[var][i];
			 }
		 }
		 for (int k = 0; k <= 2; ++k) {
			 for (int var = 0; var < numOfVar; ++var) {
				 if (k == 0) {
					load_A[var] += E[var][i];
				 } else if (k == 1) {
					load_B[var] += E[var][i];
				 } else {
					load_C[var] += E[var][i];
				 }
			 }
			 double sum = 0;
			 for (int var = 0; var < numOfVar; ++var) {
				 sum = sum + sqrt(((load_A[var] - load_B[var])*(load_A[var] - load_B[var]) + (load_C[var] - load_B[var])*(load_C[var] - load_B[var]) + (load_A[var] - load_C[var])*(load_A[var] - load_C[var]))/2);
			 }
			if (sum < sum_min) {
			   sum_min = sum;
			   saved = k;
			}
			 for (int var = 0; var < numOfVar; ++var) {
				 if (k == 0) {
					load_A[var] -= E[var][i];
				 } else if (k == 1) {
					load_B[var] -= E[var][i];
				 } else {
					load_C[var] -= E[var][i];
				 }
			 }
		 }
		 for (int var = 0; var < numOfVar; ++var) {
			 if (coeff == 0) {
				load_A[var] += E[var][i];
			 } else if (coeff == 1) {
				load_B[var] += E[var][i];
			 } else {
				load_C[var] += E[var][i];
			 }
		 }

		 if (sum_min < P.topElec(5)) {
			 P.addChromosome(chromo->local_optimal(i,saved));
		 }
		 g = g->next;
		 ++i;
	 }
}

void Mutation_ver1() {
	Chromosome *ptr = P.firstChromo;
	int idx = 0, len = P.length;

	while (idx < len) {
		//orig_value : gia tri ban dau cua chromosome
		//illu_value : gia tri anh sang cua chromosome
		//elec_value : gia tri dien nang tieu thu cua chromosome
		bool check = true;
		for (int var = 0; var < numOfVar; ++var) {
			int elec_value = ptr->electric_value[var];
			if (elec_value/total_load[var] > load_threshold) {
				check = false;
				break;
			}
		}
		if (check) {
			reducing_penalty(ptr);
		} else {
			reducing_load(ptr);
		}

		++idx;
		ptr = ptr->next;
	}
}

void Crossover() {
	Chromosome* ptr1 = P.firstChromo;
	int iCount = 0,iLen = P.length;
	while (iCount<iLen) {
		Chromosome* ptr2 = ptr1->next;
		int jCount = iCount+1;
		while (jCount<iLen) {
			//cout<<iCount<<' '<<jCount<<endl;
			Chromosome* C = ptr1->mix(ptr2);
			P.addChromosome(C);
			//C->writeAll();
			C = ptr2->mix(ptr1);
			P.addChromosome(C);
			//C->writeAll();
			ptr2 = ptr2->next;
			++jCount;
		}
		++iCount;
		ptr1 = ptr1->next;
	}
	//cout<<endl;
	//P.write();
}

void Initialization() {
	for (int iP=0; iP<50; ++iP) {
	  Chromosome* ch = new Chromosome();
	  for (int i=0; i<n; ++i) {
		  //cout << lv[i]<<' ';
		  Gene* g = new Gene(rand()%3);
		  ch->addGene(g);
		  //delete g;
	  }
	  P.addChromosome(ch);
	  //delete ch;
			//cout<<endl;

	}
	//P.write();
}

void Process_GA() {
	Initialization();
	//Termination();
	//Gene* gg = new Gene(10);
	//cout << sizeof(*gg) << ' '<< sizeof(gg->data) << ' '<< sizeof(gg->next)<<endl;
	for (int i=1;i<=10;++i) {
		 //cout << P.firstChromo->fitness <<endl;
		 //cout << P.length << endl;
		Crossover();
		//P.write();
		Mutation_ver1();
		//Mutation_ver2();

		Termination();
		//if ((clock()-start)/CLOCKS_PER_SEC > 60) break;
		//if (P.firstChromo->fitness == minFit || ((clock()-start)/CLOCKS_PER_SEC > 600)) break;
		//minFit = P.firstChromo->fitness;
	}
	//P.write();
	//cout<<fixed<<setprecision(3)<<((double)(clock()-start)/CLOCKS_PER_SEC);
	//int res = P.firstChromo->fitness;
	//fo<<res<<endl;//<<' '<<P.firstChromo->endPoint.size()<<endl;
	P.firstChromo->write();
	P.firstChromo->write_diff();
	P.firstChromo->write_load();
}

void Input(){
	//n vi tri
	//m den
	//c muc do
	//LX[i] do sang tu nhien tai vi tri i
	//E[i][j] luong dien tieu thu khi bat den i voi do sang c
	//L[i][j][k] do sang co ban cua den i, muc do k toi vi tri j
	string filename = id+"_input.txt";
	ifstream fi(filename);
	load_threshold = load_threshold / 100;
	fi>>n;
	for (int i=0; i<n; ++i) {
		fi>>x_init[i];
	}
	for (int var = 0; var < numOfVar ; ++var) {
		for (int i=0;i<n;++i) {
			fi>>E[var][i];
			//cout << E[var][i] << ' ';
			total_load[var] += E[var][i];
		}
	}
	Chromosome* ch = new Chromosome();
	for (int i=0; i<n; ++i) {
	   //cout << lv[i]<<' ';
	   Gene* g = new Gene(x_init[i]);
	   ch->addGene(g);
	}
	P.addChromosome(ch);
	//delete ch;
	//cout << ch << endl;
	//cout << &P.firstChromo;
	//cout << total_load << endl;
	//cout << P.firstChromo->fitness;
	//P.firstChromo->write_load();
}

int main(int argc, char *argv[]) {
	load_threshold = 0;
	for (char c = *argv[1]; c; c=*++argv[1]) {
		load_threshold = load_threshold*10 + (c-48);
	}
	penalty_weight = 0;
	for (char c = *argv[2]; c; c=*++argv[2]) {
		penalty_weight = penalty_weight*10 + (c-48);
	}
	id = "";
	for (char c = *argv[3]; c; c=*++argv[3]) {
		id = id + c;
	}
	//srand(time(0));
	Input();
	//Pre_Process();
	Process_GA();
}
