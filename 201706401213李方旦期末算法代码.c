#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node
{
	double upbound; //max bound of value//如果进行这个选择 所能获取的最大价值（预测）
	double value; 	//value
	int *constrain; //array[]

	int level; //height of this node in the total tree

	struct node* next;
} heapNode;	

heapNode *head,*tail;//root// queue


 

//init according to file
double *_value;//原始数据（每个物品对应的价值
double *value; //排序后的数据

int **_attribute;//原始数据 二维数组->每个物品 的 每个属性的体积
int **attribute;//排序后的数据

int num_thing;//物品数量
int num_attr;//属性数量
double check;//要求的结果//todo
int *limitation; //背包容量（数组


double bestv; //best value//当前计算出的最好的结果
int *c_limit;// current_constrain//当前装到背包的体积
double c_value;// current_value//当前价值

//排序 -》
int *order;//sorted by proportion//根据单位价值 对物品进行排序
double *proportion;//每个物品的 单位价值


void initial();
double maxBound(int t);
double mkp();
heapNode* stack_pop();
int do_something(int opr, int *l, int *target);
int stact_empty();
void addLiveNode(double upper, double cvalue, int* cons, int level);
int compare(int* target, int index);
void minus(int* target, int index);
void sort();

int main(void){
	FILE *f = fopen("test.txt","r");
	fscanf(f,"%d %d %lf",&num_thing,&num_attr,&check);
	initial();
	int i,j;
	
	//initial values
	for(i = 0; i < num_thing; i++){
		fscanf(f,"%lf",&_value[i]);
	}

	//initial attributes
	for(i = 0 ; i < num_attr;i++)
		for(j = 0; j < num_thing; j++)
			fscanf(f,"%d", &_attribute[i][j]);

	//initial limitation
	for(i = 0 ; i < num_attr;i++)
		fscanf(f,"%d", &limitation[i]);
	sort();
	double result = mkp();
	printf("my result is %lf\ntarget is %lf", result,check);

}

void initial(){
	value = (double*)malloc(sizeof(double) * num_thing);
	attribute = (int**)malloc(sizeof(int*) * num_attr);
	_value = (double*)malloc(sizeof(double) * num_thing);
	_attribute = (int**)malloc(sizeof(int*) * num_attr);
	int i;
	for(i = 0 ; i < num_attr;i++)
		*(attribute + i) = (int*)malloc(sizeof(int) * num_thing);
	for(i = 0 ; i < num_attr;i++)
		*(_attribute + i) = (int*)malloc(sizeof(int) * num_thing);

	limitation = (int*)malloc(sizeof(int) * num_attr);
	c_limit = (int*)malloc(sizeof(int) * num_attr);
	order = (int*)malloc(sizeof(int) * num_thing);
	proportion = (double*)malloc(sizeof(double) * num_thing);
	memset(c_limit,0,sizeof(int)*num_attr);
	memset(order,0,sizeof(int)*(num_thing+1));
	memset(proportion,0,sizeof(double)*num_thing); 
	head = NULL;
	tail = NULL;
}


double maxBound(int t){
	//计算（预测值）即 当前节点 下的最大预测值
	double bound = c_value;
	int *left = (int*)malloc(sizeof(int) * num_attr);
	int i,j;
	for(i = 0; i<num_attr; i++)
		*(left + i) = *(limitation + i) - *(c_limit + i);
	//从前到后 装到最满
	while(t < num_thing && compare(left, t)){
		minus(left, t);
		bound += value[t];
		t++;
	}
	//假设这个物品能分解
	if(t < num_thing){
	//	printf("%d,%d\n",l,t);
		bound += (value[t]/attribute[0][t]) * left[0];
	}

	free(left);
	return bound;

}

int do_something(int opr, int *l, int *target){
	//opr 0 compare(whether l >= target), opr 1->minus, 2->add, , 3-> find minimum weight
	int i;
	//比较l数组和target数组，是否l中每一个数都大于target中对应的数
	if(opr == 0){
		for(i = 0;i<num_attr;i++){
			 if(l[i] < target[i])
			 	return 0;
		}
	}
	//l中每一个数减去target中对应的数（计算背包剩余容量
	else if(opr == 1){
		for(i = 0;i<num_attr;i++){
			*(l+i) -=*(target+i);
		}
	}
	//加
	else if(opr == 2){
		for(i = 0;i<num_attr;i++){
			l[i]+= target[i];
		}
	
	}
	return 1;
}


double mkp(){
	int i = 0,j;
	double upbound = maxBound(i);
	
	while(1){

		int *c_weight = (int*)malloc(sizeof(int)*num_attr);
		memset(c_weight,0,sizeof(int)*num_attr);

		for(j = 0;j<num_attr;j++){
			c_weight[j] = attribute[j][i] + c_limit[j];
		}
		//左子树（要这个物品
		if(do_something(0, limitation,c_weight)){
			if(c_value + value[i] > bestv)// 预测当前节点下是否有存在最优解的可能
				bestv = c_value + value[i];
			addLiveNode(upbound,c_value + value[i], c_weight, i + 1); 
		//	printf("add!!!!");
		}
	//	printf("%lf \n",bestv);
	//	printf("upper:%lf \n",upbound);		
		upbound = maxBound(i + 1);
		//右子树（不要这个物品
		if(upbound >= bestv) // 预测当前节点下是否有存在最优解的可能
			addLiveNode(upbound, c_value, c_limit, i + 1);
		else{
			free(c_limit);
		}
		if(stact_empty()) // check stack empty 如果队列空 则 当前bestv就是我们要求的最优解
			return bestv;
		//拿出下一个要处理的节点
		heapNode *node = stack_pop(); //stack.pop()
		c_limit = node->constrain;
		c_value = node->value;
		upbound = node->upbound;
		i = node->level;
		free(node);
		node = NULL;
	}
}

//从队列中 拿出第一个元素
heapNode* stack_pop(){
	if(head == tail)
		tail == NULL;
	heapNode* item = head;
	head = item->next;
	return item;	
}
/*
	
	头 
*/


//把一个 有可能存在最优解的点 放到队列中 
void addLiveNode(double upper, double cvalue, int* cons, int level){
	heapNode *temp = (heapNode*)malloc(sizeof(heapNode));
	temp->upbound = upper;
	temp->value = cvalue;
	temp->constrain = cons;
	temp->level = level;
	temp->next = NULL;
	if(level <= num_thing){
		if(stact_empty()){
			head = temp;
			tail = temp;
		}
		else{

				tail->next = temp;
				tail= temp;	
		}
		
	}
}

int stact_empty(){
	return (head == NULL);
}

//比较 属性限制
int compare(int* target, int index){
	int i;
	for(i = 0; i < num_attr; i++){
		if(target[i] < attribute[i][index])
			return 0;
	}
	return 1;
}

//剩余容量减去这个要放入物品的重量
void minus(int* target, int index){
	int i;
	for(i = 0; i < num_attr; i++){
		target[i] -= attribute[i][index];
	}

}

void sort(){
	//calculate the proportion
	//   value/(all constration weight)
	int i,j;
	double sum;
	for(i = 0;i < num_thing;i++){
		sum = 0;
		for(j = 0;j < num_attr;j++){
			sum += ((double)attribute[j][i]/limitation[j]);
		}
		proportion[i] = value[i]/sum;
	}
	//根据每个物品 除以 这个物品对应的 所有属性之和 ->性价比
	//根据性价比高低 降序排列
	

	double temp;
	int temp_index;
	for(i = 0; i <num_thing;i++)
		order[i] = i;
	
	//bublesort

	for(j = 0;j<num_thing-1;j++){
		for(i = 0;i<num_thing-j-1;i++){
			if(proportion[i] < proportion[i+1]){
				temp = proportion[i];
				proportion[i] = proportion[i+1];
				proportion[i+1] = temp;
				temp_index = order[i];
				order[i] = order[i+1];
				order[i+1] =temp_index;
			}
		}
	}
	for(i = 0; i < num_thing;i++){
		value[i] = _value[order[i]];
		for(j = 0; j <num_attr;j++){
			attribute[j][i] = _attribute[j][order[i]];
		}
	}

}
// int find_index(int t){
// 	int i,j = 0;
// 	double min = (double)attribute[0][t]/limitation[0];
// 	for(i = 1; i < num_attr;i++){
		
// 		if((double)attribute[i][t]/limitation[i] < min&&attribute[i][t]!=0){
// 			min = (double)attribute[i][t]/limitation[i]; 	
// 			j = i;
// 		}	
// 	}
// //	printf("%lf",min); 
// 	return j;
// }
