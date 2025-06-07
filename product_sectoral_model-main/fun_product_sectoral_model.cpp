//#define EIGENLIB			// uncomment to use Eigen linear algebra library

#include "fun_head_fast.h"

// do not add Equations in this area

MODELBEGIN

// insert your equations here, ONLY between the MODELBEGIN and MODELEND words


////////////////// EXOGENOUS VARIABLES ///////////////////////


EQUATION("S_Capital_Price")
/*
Sector price of capital. Current price multiplied by a exogenous growth rate.
*/		
v[0]=VL("S_Capital_Price",1);                                                          	 
v[1]=V("S_capital_price_growth");																                    							
v[2]=v[0]*(1+v[1]);                                              					                                                                     			
RESULT(v[2])

EQUATION("S_Input_Price")
/*
Sector price of inputs. Current price multiplied by a exogenous growth rate.
*/		
v[0]=VL("S_Input_Price",1);                                                          	 
v[1]=V("S_input_price_growth");																                    							
v[2]=v[0]*(1+v[1]);                                              					                                                                  			
RESULT(v[2])

EQUATION("S_CPI")
/*
Sector CPI. Current value multiplied by a exogenous growth rate.
*/	
v[0]=VL("S_CPI",1);                                                          	 
v[1]=V("S_cpi_growth");																                    							
v[2]=v[0]*(1+v[1]);                                              					                                                                   			
RESULT(v[2])


////////////////// COST AND PRICES ///////////////////////


EQUATION("F_Wage")
/*
Firm Wage. Adjusted every period based on the growth rate of firm productivity, inflation and growth rate of sectoral employment, adjusted by the respective passthrough parameters.
*/
v[0]=VL("F_Wage",1);   
	                                                       	 			
v[3]=VL("F_Avg_Productivity", 1);                                           	 			
v[4]=VL("F_Avg_Productivity", 2);                                     			
if(v[4]!=0)
	v[5]=(v[3]-v[4])/v[4];//productivity growth                                        			
else
	v[5]=0;
		
v[6]=VL("S_CPI", 1);                                								
v[7]=VL("S_CPI", 2);                        								
if(v[7]!=0)
	v[8]=(v[6]-v[7])/v[7];//inflation                                                     			
else
	v[8]=0;
		
v[12]=VL("S_Employment", 1);                                							
v[13]=VL("S_Employment", 2);                        							
if(v[13]!=0)
	v[14]=(v[12]-v[13])/v[13];//employment growth                                                   		
else
	v[14]=0;
		
v[9]=V("S_wage_passthrough_productivity");                                       				
v[10]=V("S_wage_passthrough_inflation");														
v[11]=V("S_wage_passthrough_employment");	
v[15]=v[0]*(1+v[5]*v[9]+v[8]*v[10]+v[11]*v[14]);                                           
		
RESULT(v[15])


EQUATION("P_Variable_Cost")
/*
Product Variable Cost. Depends on the firm wage and average productivity, labour cost (the same for all products of the firm) and the input cost, which is different for each product.
*/
v[1]=V("F_Wage");
v[2]=VL("F_Avg_Productivity",1); 
v[3]=V("P_input_tech_coefficient");                               			  
v[4]=V("S_Input_Price");           
if(v[2]!=0)
	v[5]=(v[1]/v[2])+(v[3]*v[4]);
else
	v[5]=0;	
RESULT(v[5])


EQUATION("P_Desired_Markup")
/*
Desired markup over each product. If the firm has already achieved the desired market share for that product, it increases the desired markup by an adjusment parameter.
*/
v[0]=V("P_id");
v[1]=VL("P_Desired_Markup",1);                       
v[2]=V("P_Desired_Market_Share");                    
v[3]=VL("P_Market_Share",1);                      
if (v[2]>v[3])											
	{
	v[4]=V("F_markup_adjustment");
	v[5]=v[1]+v[4];				
	}												
else													
	v[5]=v[1];													
RESULT(v[5])

EQUATION("P_Desired_Market_Share")
/*
Desired markup over each product. Desired Market Share is a simple average between last period's desired market share and firm's average market share for each product.
*/
	v[0]=V("P_markup_period");								//mark up period parameter
	v[1]=fmod((t+v[0]),v[0]);								//devides the current time step by the markup period and takes the rest
	v[2]=V("P_id");						            		//product id
	v[3]=fmod((v[2]+v[0]),v[0]);							//divides the product id by the investment period and takes the rest
	v[4]=VL("P_Desired_Market_Share", 1);                  	//desired market share in the last period
	v[5]=VL("P_Avg_Market_Share", 1);                      	//product's average market share (desired)
	if(v[3]==v[1])                                         	//if the rest of the above division is zero, adjust desired market share
		v[6]=(v[4]+v[5])/2;                                 //current desired market share is a simple average between last period's desired market share and product's average market share
	else                                            		//if the rest of the above division is not zero, do not adjust desired market share
 		v[6]=v[4];                                			//product's desired market share will be equal to the last period's
RESULT(v[6])


EQUATION("P_Avg_Market_Share")
/*
Average Market Share between the market share of the firm in the last markup period
*/
	v[0]=V("P_markup_period");
	v[3]=0;										   			//initializes the sum
	for (v[1]=1; v[1]<=(v[0]-1); v[1]=v[1]+1)		    	//from 0 to markup period-1 lags
		{
		v[2]=VL("P_Market_Share", v[1]);					//computates product's market share of the current lag
		v[3]=v[3]+v[2];										//sum up product's lagged market share
		}
	v[4]=v[3]/v[0];											//average product's market share of the last investment period
RESULT(v[4])

EQUATION("P_Price")
/*
Product Price. Is a weighted average between the firm's desired price for that product (desired markup over the variable cost) and the average price of the product.
*/
v[0]=VL("P_Price",1);                                                   						
v[1]=V("P_Variable_Cost");                          											
v[2]=V("P_Desired_Markup");                         											
v[3]=V("F_strategic_price_weight");  
cur = SEARCH_CNDS(GRANDPARENT, "M_id", V("P_id"));                                 						
v[4]=VLS(cur, "M_Avg_Price", 1);                                            						
v[5]=v[3]*(v[1]*v[2])+(1-v[3])*(v[4]);                                          				                                                         						
v[6]=V("P_indirect_tax_rate");
v[7]=V("F_markup_passthrough_tax");
v[8]=v[5]/(1-(v[7]*v[6]));
RESULT(v[8])


EQUATION("P_Effective_Markup")
/*
Product Effective Markup. Effective price over variable cost.
*/
v[0]=V("P_Price");
v[1]=V("P_Variable_Cost");
if(v[1]!=0)
	v[2]=v[0]/v[1];
else
	v[2]=0;
RESULT(v[2])


////////////////// COMPETITIVENESS AND MARKET SHARE ///////////////////////


EQUATION("P_Competitiveness")
/*
Product Competitiveness. Depends negatively on price and positvely on quality.
*/
cur = SEARCH_CNDS(GRANDPARENT, "M_id", V("P_id"));                                 						 												
v[0]=VL("P_Price",1);                                          								
v[1]=VS(cur,"M_elasticity_price");                                        								
v[2]=VL("P_Quality",1);                                         								
v[3]=VS(cur,"M_elasticity_quality");                                      								
if(v[0]!=0&&v[2]!=0)                                               					
   v[7]=(pow(v[2],v[3]))*(1/pow(v[0],v[1]));   								
else                                                               								
   v[7]=CURRENT;    	                                                 								
RESULT(v[7])


EQUATION("P_Market_Share")
/*
Product Market Share. Adjusted based on the difference between firm's competitiveness and the average competitiveness of the market. Firms above the average gain market share, firms below the average lose market share.
*/
v[0]=VL("P_Market_Share", 1);      
cur = SEARCH_CNDS(GRANDPARENT, "M_id", V("P_id"));                                 						 												
v[1]=VS(cur,"M_Avg_Competitiveness");          													
v[2]=V("S_competitiveness_adjustment");										
v[3]=V("P_Competitiveness");                 												
if(v[1]!=0)                                     												
	v[4]=v[0]+v[2]*v[0]*((v[3]/v[1])-1);             												
else                                            												
	v[4]=0;                                     												
RESULT(v[4])


////////////////// PRODUCTION ///////////////////////


EQUATION("P_Expected_Sales")
/*
Product Expected Sales. Simple adaptative expectational rules based on past demand.
*/
v[1]=VL("P_Effective_Orders", 1);                    										
v[2]=VL("P_Effective_Orders", 2);                    										
v[3]=V("F_expectations");                            												
if(v[2]!=0)                                           											
	v[4]=max(0,v[1]*(1+v[3]*((v[1]-v[2])/v[2])));              										
else                                                  											
	v[4]=v[1]; 	                                        											
RESULT(v[4])


EQUATION("P_Planned_Production")
/*
Product Planned Production. Based on expected sales considering existing and desired inventories.
*/
v[1]=V("P_Expected_Sales");                          										
v[2]=V("P_desired_inventories_proportion");             											
v[3]=VL("P_Inventories",1);                    										
v[4]=v[1]*(1+v[2])-v[3];                            											
RESULT(v[4])


EQUATION("F_Effective_Production")
/*
Firm Effective Production. Firm will calculate the planned production for all products and consider the entire productive capacity. Higher profit rate produts will be produced first. More productive capital goods will be used first.
*/
CYCLE(cur, "CAPITAL")
	{
	WRITES(cur, "C_production", 0 );							// intitilize the production of each capital good as zero.
	}

SORTL("PRODUCT", "P_Profit_Rate", "DOWN", 1);					// sort products by past profit rates
v[7]=0;
CYCLE(cur, "PRODUCT") 											// cycle through all products                                                                      
	{
	v[0]=VS(cur,"P_Planned_Production");                                                              			
	SORT("CAPITAL", "C_productivity", "DOWN");   				//sort capital goods by productivity                                     
	v[1]=0;                                                                                      	
		CYCLES(PARENT, cur1, "CAPITAL")  						// cycle trough all capital goods                                                                       
		{
		v[2]=VS(cur1, "C_productive_capacity");
		v[3]=VS(cur1, "C_production");                       
		v[4]=max(0,(min(v[2]-v[3], v[0])));                     //for each capital good, it produced the minimum between the planned production and the capital productive capacity                                                    	
		INCRS(cur1, "C_production", v[4]);                      //write the capital good product                                     
		v[0]=v[0]-v[4];                                                                            	
		v[1]=v[1]+v[4];                                                                            	
		}
	WRITES(cur, "P_Effective_Production", v[1]);
	v[7]=v[7]+v[1];
	}

RESULT(v[7])
EQUATION_DUMMY("P_Effective_Production", "F_Effective_Production")


EQUATION("F_Avg_Productivity")
/*
Firm Avg Productivity. To be used in the variable cost calculation. Weighted average of each capital good productivity weighted by the production.
*/                            							                   									
v[2]=0;                                                        									
v[3]=0;                                                        									
CYCLE(cur, "CAPITAL")                                          								
	{
	v[4]=VS(cur, "C_productivity");                   								
	v[5]=VS(cur, "C_production");                    								
	v[2]=v[2]+v[4]*v[5];                                        								
	v[3]=v[3]+v[5];                                             								
	}
if (v[3]!=0)                                                   									
	v[6]=v[2]/v[3];                                             								
else                                                           									
	v[6]=CURRENT;  	                                                 								
RESULT(v[6])


EQUATION("F_Capacity_Utilization")
/*
Firm Capacity Utilization. Effective Production over total productive capacity
*/
v[0]=V("F_Effective_Production");
v[1]=SUM("C_productive_capacity");
if(v[1]!=0)
		v[2]=v[0]/v[1];
else
		v[2]=0;	
RESULT(v[2])


////////////////// DEMAND AND SALES ///////////////////////


EQUATION("M_Effective_Orders")
/*
Market Effective Orders. Total Demand.
*/
v[0]=V("M_date_birth");
v[1]=V("S_demand_scale");
v[2]=V("S_logistic_growth");
v[3]=V("S_demand_duration");	
v[4]=1/(1 + exp(-v[2] * (t - v[0] - v[3])));            										 //rising logistically
v[5]=1/(1 + exp( v[2] * (t - v[0] - 2*v[3])));    										     //declining logistically
v[6]=v[1]*v[4]*v[5];
RESULT(v[6])


EQUATION("P_Effective_Orders")
/*
Product Effective Orders. Market effective orders distributed by the market share.
*/
v[1]=V("P_Market_Share");
cur = SEARCH_CNDS(GRANDPARENT, "M_id", V("P_id"));                                 						 												
v[2]=VS(cur, "M_Effective_Orders");
v[3]=v[1]*v[2];
RESULT(v[3])


EQUATION("P_Sales")
/*
Product Sales. Minimum between supply (production and inventories) and demand.
*/
v[0]=VL("P_Inventories",1);                   												
v[1]=V("P_Effective_Production");                   												
v[2]=V("P_Effective_Orders");                       												
v[3]=v[0]+v[1];                                        												
v[4]=max(0, min(v[3],v[2]));                           												
RESULT(v[4])


EQUATION("P_Inventories")
/*
Product Inventories. Adjusted by the difference between sales and production.
*/
v[0]=V("P_Effective_Production");                    											            
v[1]=V("P_Sales");                                   											
v[2]=VL("P_Inventories", 1);                  												
v[3]=v[0]-v[1]+v[2];                                    											
RESULT(v[3])


EQUATION("P_Effective_Market_Share")
/*
Product Effective Market Share. Firm Sales over Market sales.
*/
v[0]=V("P_Sales");
cur = SEARCH_CNDS(GRANDPARENT, "M_id", V("P_id"));                                 						 												
v[1]=VS(cur, "M_Sales");
if (v[1]!=0)
	v[2]=v[0]/v[1];
else
	v[2]=0;	
RESULT(v[2])


EQUATION("P_Revenue")
/*
Product Revenue. Sales times price.
*/
v[0]=V("P_Sales");                                             											
v[1]=V("P_Price");                                             											
v[2]=v[0]*v[1];	
RESULT(v[2])


EQUATION("P_Indirect_Tax")
/*
Product Tax. Product indirect tax rate over the product revenue.
*/
v[0]=V("P_Revenue");
v[1]=V("P_indirect_tax_rate");
v[2]=v[0]*v[1];
RESULT(v[2])


EQUATION("P_Aftertax_Revenue")
/*
Product After Tax Revenue. Product revenue minus taxes
*/
v[0]=V("P_Revenue");
v[1]=V("P_indirect_tax_rate");
v[2]=v[0]*(1-v[1]);
RESULT(v[2])


////////////////// PROFITS ///////////////////////



EQUATION("P_Profits")
/*
Product Profit. Revenue minus tax minus production costs. To be used for profit rate calculation. Not for proper firm accounting.
*/
v[0]=V("P_Revenue");
v[1]=V("P_indirect_tax_rate");
v[2]=V("P_Effective_Production");
v[3]=V("P_Variable_Cost");
v[4]=v[0]*(1-v[1])-v[2]*v[3];
RESULT(v[4])


EQUATION("P_Profit_Rate")
/*
Product Profit Rate. Product profit over firm capital.
*/
v[0]=V("P_Profits");
v[1]=VL("F_Capital",1);
if(v[1]!=0)
	v[2]=v[0]/v[1];
else
	v[2]=0;	
RESULT(v[2])


EQUATION("P_Quality_RND_Expenses")
/*
Product Quality RND expenses. Share of aftertax revenue allocated to quality rnd
*/
v[0]=V("P_Aftertax_Revenue");
v[1]=V("P_Quality_RND_Share");    // endogenised
v[2]=v[0]*v[1];
RESULT(v[2])

EQUATION("P_Quality_RND_Share")
/*
This variable defines the share of expenses of quality rnd.
If the firm is gaining market share, it decreases quality rnd. The opposite is also true.
*/

	v[1]=V("P_Desired_Market_Share");               //desired market-share 
	v[2]=VL("P_Avg_Market_Share",1);                //product average market share in the last period   
	v[3]=VL("P_Quality_RND_Share",1);		   		//last period share
	v[4]=V("P_quality_adjustment");		    		//adjustment parameter							

	if (v[1]>v[2])									//if current avg market share is lower than desired												
		v[5]=v[3]+v[4];								//increases quality rnd share
	if (v[1]<v[2])									//if current avg market share is higher than desired
		v[5]=v[3]-v[4];								//reduces quality rnd share
	else
		v[5]=v[3];	
	v[6]=V("P_quality_rnd_share_max");                 
	v[7]=max(0,min(v[5],v[6]));
	
RESULT(v[7])

EQUATION("F_Interest_Payment")
/*
Firm Interest Payment. Interest on existing stock of debt.
*/
v[0]=V("F_Interest_Rate");
v[1]=VL("F_Stock_Debt",1);
v[2]=v[0]*v[1];	                                   			               
RESULT(v[2])								


EQUATION("F_Debt_Payment")
/*
Firm Debt Payment/ Amortization on existing stock of debt.
*/
v[0]=V("F_debt_repayment");
v[1]=VL("F_Stock_Debt",1);
v[2]=v[0]*v[1];	
RESULT(v[2])


EQUATION("F_Financial_Obligations")
/*
Firm financial obligations. Interest payments plus amortization.
*/
v[1]=V("F_Interest_Payment");
v[2]=V("F_Debt_Payment");
v[3]=v[1]+v[2]; 
RESULT(v[3])


EQUATION("F_Productivity_RND_Expenses")
/*
Firm productivity rnd expenses. Share of total aftertax revenue allocated to productivity rnd.
*/
v[1]=SUM("P_Aftertax_Revenue");
v[2]=V("F_productivity_rnd_proportion");    
v[3]=v[1]*v[2]; 
RESULT(v[3])



EQUATION("P_Product_Innovation_RND_Expenses")
/*
Product RND expenses. Share of aftertax revenue allocated to product rnd
*/
v[0]=V("P_Aftertax_Revenue");
v[1]=V("P_Product_RND_Share");    // endogenised
v[2]=V("P_Innovation_RND_Share");
v[3]=v[0]*v[1]*v[2];
RESULT(v[3])

EQUATION("P_Product_Imitation_RND_Expenses")
/*
Product RND expenses. Share of aftertax revenue allocated to product rnd
*/
v[0]=V("P_Aftertax_Revenue");
v[1]=V("P_Product_RND_Share");    // endogenised
v[2]=V("P_Innovation_RND_Share");
v[3]=v[0]*v[1]*(1-v[2]);
RESULT(v[3])

EQUATION("P_Product_RND_Share")

	v[1]=V("P_Desired_Market_Share");               //desired market-share 
	v[2]=VL("P_Avg_Market_Share",1);                //product average market share in the last period   
	v[3]=VL("P_Product_RND_Share",1);		   		//last period share
	v[4]=V("P_product_adjustment");		    		//adjustment parameter							

	if (v[1]>v[2])									//if current avg market share is lower than desired												
		v[5]=v[3]+v[4];								//increases product rnd share
	if (v[1]<v[2])									//if current avg market share is higher than desired
		v[5]=v[3]-v[4];								//reduces product rnd share
	else
		v[5]=v[3];	
	v[6]=V("P_product_rnd_share_max");                 
	v[7]=max(0,min(v[5],v[6]));

RESULT(v[7])

EQUATION("P_Innovation_RND_Share")
/*
This variable defines the allocation of expenses between innovation and imitation.
If the firm is gaining market share, it decreases imitation and increases innovation. The opposite is valid.
*/

	v[1]=V("P_Desired_Market_Share");                //desired market-share 
	v[2]=VL("P_Avg_Market_Share",1);                 //average market share in the last period   
	v[3]=VL("P_Innovation_RND_Share",1);				//last period share (initial alocation is 0.5)
	v[4]=V("P_innovation_imitation_adjustment");			//adjustment parameter							

	if (v[2]>v[1])										//if current avg market share is lower than desired												
		v[5]=v[3]-v[4];									//reduces innovation rnd allocation
	if (v[2]<v[1])										//if current avg market share is higher than desired
		v[5]=v[3]+v[4];									//increases innovation rnd allocation
	if (v[1]==v[2])
		v[5]=v[3];	
		                
	v[6]=max(0,min(v[5],1));
RESULT(v[6])


EQUATION("F_Product_Innovation_RND_Expenses")
/*
Firm product innovation rnd expenses. Share of total aftertax revenue allocated to product innovation rnd.
*/
v[1]=SUM("P_Product_Innovation_RND_Expenses");
RESULT(v[1])


EQUATION("F_Product_Imitation_RND_Expenses")
/*
Firm product imitation rnd expenses. Share of total aftertax revenue allocated to product imitation rnd.
*/
v[1]=SUM("P_Product_Imitation_RND_Expenses");
RESULT(v[1])


EQUATION("F_Net_Profits")
/*
Firm Net profits. Sum of all products aftertax revenue, minus all products quality rnd expenses, minutes productivity rnd expenses minus product rnd expenses, minus financial obligations, minus the production costs of all products.
*/
v[0]=SUM("P_Aftertax_Revenue");                                       											
v[1]=SUM("P_Quality_RND_Expenses");                              											
v[2]=V("F_Productivity_RND_Expenses");                                     											
v[3]=V("F_Product_Innovation_RND_Expenses");
v[4]=V("F_Product_Imitation_RND_Expenses");							  												
v[5]=V("F_Financial_Obligations");
v[6]=WHTAVE( "P_Effective_Production", "P_Variable_Cost" );
v[7]=v[0]-v[1]-v[2]-v[3]-v[4]-v[5]-v[6];								  									
RESULT(v[7])


EQUATION("F_Distributed_Profits")
/*
Firm distributed profits. Fixed share of total profits, only if positive.
*/
v[0]=V("F_Net_Profits");                                        											          
v[1]=V("F_profits_distribution_rate");                               											
if(v[0]>0)                                                         											
	v[2]=v[1]*v[0];                                                											
else                                                               											                                                                  
	v[2]=0;																											
RESULT(v[2])


EQUATION("F_Retained_Profits")
/*
Firm retained profits. To be used as funds for investmemt. If profits are negative, they are all retained.
*/
v[0]=V("F_Net_Profits");                                        											       
v[1]=V("F_profits_distribution_rate");                               											                           
if(v[0]>0)                                                         											
	v[2]=(1-v[1])*v[0];                                           											
else                                                               											                                                                    
	v[2]=v[0];                                                     											                                                         
RESULT(v[2])


EQUATION("F_Profit_Rate")
/*
Firm Profit Rate. Firm profits over firm capital
*/
v[0]=V("F_Net_Profits");
v[1]=VL("F_Capital",1);
if(v[1]!=0)
	v[2]=v[0]/v[1];
else
	v[2]=0;	
RESULT(v[2])


////////////////// R&D ///////////////////////


EQUATION("F_Frontier_Productivity")
/*
Firm frontier productivity. Maximum between existing one, innovation one and imitation one.
*/
v[0]=VL("F_Frontier_Productivity", 1);      
v[1]=V("F_Imitation_Productivity");         
v[2]=V("F_Innovation_Productivity");         
v[3]=max(v[0],max(v[1],v[2]));             		
RESULT(v[3])


EQUATION("F_Imitation_Productivity")
/*
Firm imitation productivity. Depends on the imitation success. If succeeded, copies the maximum existing productivity of other firms. Sucess depends on the resources allocated.
*/
v[0]=V("F_Productivity_RND_Expenses");               
v[1]=V("F_Wage"); 		
v[2]=V("F_productivity_innovation_share");		
v[3]=(v[0]/v[1])*(1-v[2]);                    
v[4]=1-exp(-v[3]);                   		
if(RND<v[4])                              	
     v[5]=MAXLS(PARENT, "F_Avg_Productivity", 1);  
else                                      	
     v[5]=0;     	                              
RESULT(v[5])

EQUATION("F_Innovation_Productivity")
/*
Firm innovation productivity. Depends on the innovation success. If succeeded, obtain a new random productivity. Sucess depends on the resources allocated.
*/
v[0]=V("F_Productivity_RND_Expenses");               
v[1]=V("F_Wage"); 		
v[2]=V("F_productivity_innovation_share");		
v[3]=(v[0]/v[1])*(v[2]);                    
v[4]=1-exp(-v[3]);                   		
if(RND<v[4])  
	{
	v[5]=V("S_std_dev_productivity");           
	v[6]=V("S_initial_productivity");			
	v[7]=V("S_tech_opportunity_productivity");
	v[8]=log(v[6])+(double)t*(v[7]);   		
	v[9]=exp(norm(v[8],v[5]));              
	}
else                                        
	v[9]=0;                                 
RESULT(v[9])


EQUATION("P_Quality")
/*
Product frontier quality. Maximum between existing one, innovation one and imitation one.
*/
v[0]=VL("P_Quality", 1);       				
v[1]=V("P_Imitation_Quality");          		
v[2]=V("P_Innovation_Quality");         		
v[3]=max(v[0],max(v[1],v[2]));        			
RESULT(v[3])


EQUATION("P_Imitation_Quality")
/*
Product imitation quality. Depends on the imitation success. If succeeded, copies the maximum existing quality of other firms. Sucess depends on the resources allocated.
*/
v[0]=V("P_Quality_RND_Expenses");               
v[1]=V("F_Wage"); 		
v[2]=V("P_Innovation_RND_Share");		
v[3]=(v[0]/v[1])*(1-v[2]);                    
v[4]=1-exp(-v[3]);                   		
if(RND<v[4]) 
	{                             	
     v[5]=V("P_id");
     v[8]=0;
     	CYCLES(GRANDPARENT, cur, "FIRM")
     	{
     		CYCLES(cur, cur1, "PRODUCT")
     		{
     			v[6]=VLS(cur1, "P_Quality", 1);	
     			v[7]=VS(cur1, "P_id");
     			if(v[6]>v[8]&&v[7]==v[5])
     				v[8] = v[6];
     			else
     				v[8] = v[8];
     		}
     	}
    }
else     	                                 	
     v[8]=0;       	                              
RESULT(v[8])


EQUATION("P_Innovation_Quality")
/*
Product innovation quality. Depends on the innovation success. If succeeded, obtain a new random quality. Sucess depends on the resources allocated.
*/
v[0]=V("P_Quality_RND_Expenses");               
v[1]=V("F_Wage"); 		
v[2]=V("P_Innovation_RND_Share");		
v[3]=(v[0]/v[1])*(v[2]);                    
v[4]=1-exp(-v[3]);                   		
if(RND<v[4])  
	{
	v[5]=V("S_std_dev_quality");           
	v[6]=V("S_initial_quality");			
	v[7]=V("S_tech_opportunity_quality");
	v[8]=log(v[6])+(double)t*(v[7]);   		
	v[9]=exp(norm(v[8],v[5]));              
	}
else                                        
	v[9]=0;                                 
RESULT(v[9])


EQUATION("F_Innovation_Product")
/*
Firm product innovation. Depends on the sucess of the innovation process, which in turn depends on the resources allocated. If success, firm creates a new product.
*/
v[0]=V("F_Product_Innovation_RND_Expenses");          
v[1]=V("F_Wage"); 			
v[2]=(v[0]/v[1]); 
v[3]=V("S_product_difficulty");                   
v[4]=1-exp(-v[2]*v[3]);                   		
if(RND<v[4])  
	{
	cur = RNDDRAW_FAIR( "PRODUCT" );                   //select a random product already being produced by the firm
	v[6] = VS(cur, "P_id");                            //record the product id of the selected product
	cur1 = SEARCH_CNDS(PARENT, "M_id", v[6]);          //find the market object which the market id matches the recorded product id
	cur2 = ADDOBJ_EXS(PARENT, "MARKET", cur1);         //create a new market object using the selected one as example
	v[7] = MAXS(PARENT, "M_id");                       //find the maximum existing market id
	v[8] = v[7]+1;                                     //v[8] is the maximum existing market id plus one 
	WRITES(cur2, "M_id", v[8]);                        //write the new id for the recently created market object
	WRITES(cur2, "M_date_birth", t);                   //write the data of birth of the recently created market object
	WRITELLS(cur2, "M_Effective_Orders", 0, t, 1);             //write the initial demand of the recently created market object
	WRITELLS(cur2, "M_Effective_Orders", 0, t, 0);             //write the initial demand of the recently created market object
	cur3 = ADDOBJ_EX("PRODUCT", cur);                  //create a new product object usinng the randomly selected on as example
	WRITES(cur3, "P_id", v[8]);                        //write the new id for the recently created product object
	WRITES(cur3, "P_Market_Share", 1);                 //write the market share of the recently created product object
	WRITES(cur3, "P_Effective_Orders", 0);             //write the initial demand of the recently created product object 
	}
else                                        
	v[8]=0;                                 
RESULT(v[8])


EQUATION("F_Imitation_Product")
/*
Firm product imitation. Dpends on the sucess of the imitation process, which in turn depends on the resources allocated. If success, firm starts producing a new (but already existing) product)
*/     
v[0]=V("F_Product_Imitation_RND_Expenses");         
v[1]=V("F_Wage"); 			
v[2]=(v[0]/v[1]); 
v[3]=V("S_product_patent");                   
v[4]=1-exp(-v[2]);                   		
if(RND<v[4])  
	{
	v[6]=V("F_id");                                                     //record original firm id
	v[11]=0;                                                            //initialized the cycle counter
	SORTS(PARENT, "FIRM", "F_Profit_Rate", UP);                         //sort all firms by profit rate, most profitable firms will be the last ones
	CYCLES(PARENT, cur, "FIRM")                                         //cycle through all firms
		{
		v[7]=VS(cur, "F_id");                                           //record current firm id
		if(v[6]!=v[7])   
        	{                                      					    //if current firm id is different that original firm id
			SORTS(cur, "PRODUCT", "P_Profit_Rate", UP);                 //sort all products of the current firm by profit rate                   
			CYCLES(cur, cur1, "PRODUCT")                                // cycle trought allm products of the current firm
				{
				v[9]=VS(cur1, "P_id");                                  //record product id
				cur5 = SEARCH_CNDS(THIS,"P_id",v[9]);                   //search the original firm if it alreayd produces a product with the same id
				if(cur5 == NULL)                                        //if not, record the current product id in the cycler coutner
					v[11]=v[9];
				else                                                    //if yes, keep the current values of the cycle counter.
					v[11]=v[11];			
				}
			}
		}
	
	if(v[11]!=0)                                                        //if the cycle counter is different than zero (a product was found)                              
		{
		cur1 = SEARCH_CNDS(PARENT, "M_id", v[11]);	                    //search the market object with the market id equal to the found product id
		v[13]= VS(cur1, "M_date_birth");                                //record the date of birth of the existing market
		if(t>v[13]+v[3])                                                //if the time period is geater than the birth date + patent period
			{			 
			cur2 = SEARCH_CNDS(PARENT, "P_id", v[11]);		            //search a product object with the same product id using the found object as an example
			cur3 = ADDOBJ_EX("PRODUCT", cur2);                 		    //create a new product object usinng the randomly selected on as example
			WRITES(cur3, "P_id", v[11]);                                //write the new product id of the recently created product object.
			WRITES(cur3, "P_Market_Share", 0);                          //write the new market share of the recently created product object.
			WRITES(cur3, "P_Effective_Orders", 0);                      //write the initial demand of the recently created product object.
			v[12] = v[11];
			}
		else
			v[12]=0;
		}
	else
		v[12]=0;  		  
	}
else                                        
	v[12]=0;                                 
RESULT(v[12])


////////////////// INVESTMENT AND FINANCE ///////////////////////


EQUATION("F_Productive_Capacity_Depreciation")

	v[0]=0;																				
	v[1]=COUNT("CAPITAL");
	CYCLE_SAFE(cur1, "CAPITAL")
	{
		v[3]=VS(cur1, "C_productive_capacity");								
		v[5]=VS(cur1, "C_depreciation_period");
		if((double)t>=v[5]&&v[1]>1)												
			{
			v[0]=v[0]+v[3];																
			DELETE(cur1);															
			v[1]=v[1]-1;
			}
		else																			
			{
			if((double)t>=v[5]&&v[1]<=1)
				{
				WRITES(cur1, "C_productive_capacity", 0);
				WRITES(cur1, "C_productivity", 0);
				}
			else
				v[0]=v[0];																	
			}
			
	}                                                                          
RESULT(v[0])


EQUATION("F_Productive_Capacity")

	
	v[1]=VL("F_Frontier_Productivity",1);									
	v[2]=VL("F_Demand_Capital_Goods_Expansion",1);	
	v[3]=VL("F_Demand_Capital_Goods_Replacement",1);	
	v[4]=v[2]+v[3];								
	v[5]=V("S_capital_output_ratio");
	v[6]=COUNT("CAPITAL");										
	v[8]=V("S_depreciation_period");
	
	if(v[2]>0)
		{
		for(i=0; i<=v[2]; i++)													
			{
			cur=ADDOBJ("CAPITAL");												
			WRITES(cur, "C_productivity", v[1]);				
			WRITES(cur, "C_productive_capacity", (1/v[5]));			
			WRITES(cur, "C_date_birth", t);							
			WRITES(cur, "C_to_replace", 0);							
			WRITES(cur, "C_depreciation_period", (t+v[8]));
			
			v[6]=v[6]+1;
			}																																		
		}
		
	if(v[3]>0)
		{
		SORT("CAPITAL","C_productivity","UP");
		CYCLE(cur, "CAPITAL")																																					
   			{
     		v[9]=VS(cur, "C_to_replace");
     		if(v[9]==1)
     			{
     			WRITES(cur, "C_productivity", v[1]);				
				WRITES(cur, "C_date_birth", t);							
				WRITES(cur, "C_to_replace", 0);							
				WRITES(cur, "C_depreciation_period", (t+v[8]));
     			}																																							
  			}
  		}
													
	if (v[6]!=0)																																					
		v[15] = SUM("C_productive_capacity");						
	else																																							
		v[15]=0;
																																		
RESULT(v[15])


EQUATION("F_Capital")
	v[0]=COUNT("CAPITAL");                       								
	v[1]=V("S_Capital_Price");                       						
	v[2]=v[0]*v[1];                                 							
RESULT(v[2])


EQUATION("F_Expected_Sales")
	v[0] = SUM("P_Expected_Sales")
RESULT(v[0])


EQUATION("F_Desired_Expansion_Investment_Expenses")

   		v[0]=V("F_Expected_Sales");																								 	
   		v[1]=V("F_Productive_Capacity"); 						
		
		v[10]=0;													
		v[11]=V("S_depreciation_period");								
		
		CYCLE(cur, "CAPITAL")										
		{
			v[15]=VS(cur, "C_depreciation_period");
			v[13]=VS(cur, "C_date_birth");				
			v[14]=VS(cur, "C_productive_capacity");		
			if((double)t!=v[13] && ((double)t+1) > v[15])	
				v[10]=v[10]+v[14];									
			else													
				v[10]=v[10];										
		}
																										
   		v[3]=V("F_desired_degree_capacity_utilization");
		v[4]=((v[0])/v[3]) - (v[1]-v[10]);							
   		v[5]=max(0,v[4]);											
   		
		v[6]=V("S_Capital_Price");								
		v[7]=V("S_capital_output_ratio");								
   		v[8]=v[5]*v[6]*v[7];									
   		
RESULT(v[8])


EQUATION("F_Desired_Replacement_Investment_Expenses")

 v[8]=V("F_Frontier_Productivity");
 v[9]=V("S_depreciation_period");
 v[10]=V("F_Wage");
 v[11]=V("S_Capital_Price");
 v[12]=V("S_capital_output_ratio");
 v[13]=V("S_payback_period");
  				
  v[16]=0;																																												
   CYCLE(cur, "CAPITAL")																																					
   {
     v[19]=VS(cur, "C_depreciation_period");																										
     if((double)t!=v[19] && (double)t < v[19])																					
     {
     v[17]=VS(cur, "C_productivity");																									
     v[18]=VS(cur, "C_productive_capacity");   																				 
     v[23]=v[11]/(v[10]*((1/(v[17]))-(1/(v[8]))));																								          
         if(v[8]>v[17] && v[23]<=v[13]*v[9])
         {																														
        	v[16]=v[16]+v[18];      
        	WRITES(cur, "C_to_replace",1);		
         }   																															
          else
          {																																											
         	v[16]=v[16]; 
         	WRITES(cur, "C_to_replace",0);	
          }																																							  
      }
      else
      {																																												
      	v[16]=v[16]; 	
      	WRITES(cur, "C_to_replace",0);		
      }																																						
   }
  																																											
  
  v[17]=v[16]*v[11]*v[12];																																			
RESULT(v[17])


EQUATION("F_Investment_Expenses")
v[0]=V("F_Desired_Expansion_Investment_Expenses");
v[1]=V("F_Desired_Replacement_Investment_Expenses");
v[2]=v[0]+v[1];
RESULT(v[2])

EQUATION("F_Demand_Capital_Goods_Expansion")
						
		v[2]=V("F_Desired_Expansion_Investment_Expenses");	
		v[3]=V("S_Capital_Price");						
		v[4]=v[2]/v[3];																																																
		v[5]=V("S_capital_output_ratio");														
		v[6]=v[5]*v[4];																			
	
RESULT(v[4])

EQUATION("F_Demand_Capital_Goods_Replacement")
						
		v[2]=V("F_Desired_Replacement_Investment_Expenses");	
		v[3]=V("S_Capital_Price");						
		v[4]=v[2]/v[3];																																																
		v[5]=V("S_capital_output_ratio");														
		v[6]=v[5]*v[4];																			
	
RESULT(v[4])

EQUATION("F_Stock_Debt")

	v[0]=VL("F_Stock_Debt",1);
	v[1]=V("F_Debt_Payment");
	v[2]=V("F_Retained_Profits");
	v[3]=V("F_Investment_Expenses");
	v[4]=v[0]-v[1]-v[2]+v[3];

RESULT(v[4])


////////////////// MARKET VARIABLES ///////////////////////



EQUATION("M_Avg_Price")
/*
Market Avg Price. Avergare of each firm price weighted by the market share.
*/
v[0]=V("M_id");
v[1]=0;
CYCLES(PARENT, cur, "FIRM")
     {
     CYCLES(cur, cur1, "PRODUCT")
     	{
     	v[6]=VS(cur1, "P_Price");	
     	v[7]=VS(cur1, "P_Market_Share");
     	v[8]=VS(cur1, "P_id");
     	if(v[8]==v[0])
     		v[1] = v[1] + v[6]*v[7];
     	else
     		v[1] = v[1];
     	}
     }	
RESULT(v[1])


EQUATION("M_Avg_Competitiveness")
/*
Market Avg Competitiveness. Avergare of each firm competitiveness weighted by the market share.
*/
v[0]=V("M_id");
v[1]=0;
CYCLES(PARENT, cur, "FIRM")
     {
     CYCLES(cur, cur1, "PRODUCT")
     	{
     	v[6]=VS(cur1, "P_Competitiveness");	
     	v[7]=VLS(cur1, "P_Market_Share", 1);
     	v[8]=VS(cur1, "P_id");
     	if(v[8]==v[0])
     		v[1] = v[1] + v[6]*v[7];
     	else
     		v[1] = v[1];
     	}
     }	
RESULT(v[1])


EQUATION("M_Sales")
/*
Market Sales. Sum of all firms sales.
*/
v[0]=V("M_id");
v[1]=0;
CYCLES(PARENT, cur, "FIRM")
     {
     CYCLES(cur, cur1, "PRODUCT")
     	{
     	v[6]=VS(cur1, "P_Sales");	
     	v[8]=VS(cur1, "P_id");
     	if(v[8]==v[0])
     		v[1] = v[1] + v[6];
     	else
     		v[1] = v[1];
     	}
     }
RESULT(v[1])

EQUATION("M_Avg_Sales")
/*
Market Sales. Sum of all firms sales.
*/
v[0]=V("M_id");
v[1]=0;
CYCLES(PARENT, cur, "FIRM")
     {
     CYCLES(cur, cur1, "PRODUCT")
     	{
     	v[6]=VS(cur1, "P_Sales");
     	v[7]=VLS(cur1, "P_Market_Share", 1);	
     	v[8]=VS(cur1, "P_id");
     	if(v[8]==v[0])
     		v[1] = v[1] + v[6]*v[7];
     	else
     		v[1] = v[1];
     	}
     }
RESULT(v[1])

EQUATION("S_Employment")
/*
Sector Employment. Sum of labour employed in all firms.
*/
v[0]=0;                                        		
CYCLE(cur, "FIRM")                            		
	{
	v[1]=VS(cur, "F_Effective_Production");     
	v[2]=VS(cur, "F_Avg_Productivity");   		
	if(v[2]!=0)
		v[0]=v[0]+v[1];                       		
	else
		v[0]=v[0];
	}
RESULT(v[0])

//EQUATION("M_Market_Share_Adjustment")
//V("F_Innovation_Product");
//V("F_Imitation_Product");
//V("M_Firm_Entry");

//	v[0]=V("M_id");
//	v[1]=0;
//	CYCLES(PARENT, cur, "FIRM");
//	{
//		CYCLES(cur, cur1,"PRODUCT")
//		{
//		v[2]=VS(cur1,"P_id");
//		v[3]=VS(cur1,"P_Market_Share");
//		if (v[2]==v[0])
//		v[1]=v[1]+v[3];
//		else
//		v[1]=v[1];
//		}
//	}
//	v[4]=0;
//	CYCLES(PARENT, cur2, "FIRM");
//	{
//		CYCLES(cur2, cur3,"PRODUCT")
//		{
//		v[5]=VS(cur3,"P_id");
//		v[6]=VS(cur3,"P_Market_Share");
//		if (v[5]==v[0])	
//			{
//			WRITES(cur3,"P_Market_Share",v[6]/v[1]);
//			v[4]=v[4]+v[6]/v[1];
//			}
//		else
//		v[4]=v[4];
//		}
//	}
//RESULT(v[4])

/////////////////ENTRY AND EXIT/////////////////////////////////////


EQUATION("M_Number_Potential_Entrants")
/*
Number of potential entrants.
*/
V("F_Imitation_Product");
v[0]=V("M_switch_entry");
v[1]= V("M_Effective_Orders");
v[2]=V("S_demand_scale");
v[3]=AVES(PARENT,"F_Productive_Capacity");
v[4]=V("M_max_entry_number");
v[5]= ((v[2] - v[1])/v[3]);
v[6]= round(v[5]);

     if (v[0]!=0 && v[6]>0)
     v[7]=min(v[2],v[6]);
     else
     v[7]=0;
             
RESULT(v[7])

EQUATION("M_Firm_Entry")
/*
Firm entry process into each market
*/
v[0]=V("M_id");
v[1]=V("M_Number_Potential_Entrants");
v[2]=0;
for (i=1;i<=v[1];i++)
	{
	v[3]=V("M_Effective_Orders");
	v[4]=VL("M_Effective_Orders",1);
	//v[5]=AVES(PARENT,"F_Productive_Capacity");
	v[6]=((v[3]-v[4])/v[4]);
	v[7]=V("M_entry_barrier");
	v[8]=(v[6]*v[7]);
	v[9]=bernoulli(v[8]);
		if (RND<v[9]) 									
	v[2]=v[2]+1;
	else
	v[2]=v[2];
	}
v[10]=COUNTS(PARENT,"FIRM");	
v[11]=V("M_Avg_Competitiveness");
v[12]=V("M_Avg_Price");
v[13]=VL("M_Avg_Sales",1);
	
for (i=1;i<=v[2];i++)
{
cur1=RNDDRAW_FAIRS(PARENT, "FIRM");
cur2=ADDOBJ_EXS(PARENT,"FIRM",cur1);
	WRITES(cur2,"F_id", v[10]+1),	
	WRITES(cur2,"F_date_birth",t);
	WRITELS(cur2,"F_Frontier_Productivity",1,t);
	WRITELS(cur2,"F_Capital",100,t);
	WRITELS(cur2,"F_Demand_Capital_Goods_Expansion",0,t);
	WRITELS(cur2,"F_Demand_Capital_Goods_Replacement",0,t);
	
	
	v[14]=1;
	CYCLE_SAFES(cur2,cur3, "PRODUCT")
	{
		if (v[14]>1)
		{
		DELETE(cur3);
		v[14]=v[14]+1;
		}
		else
		{
		WRITES(cur3,"P_id",v[0]);
		WRITES(cur3,"P_Price",norm(v[12],0.5));
		WRITELS(cur3,"P_Market_Share",norm(0.1,0.05),t);
		WRITELS(cur3,"P_Competitiveness",norm(v[11],0.1),t);
		WRITELS(cur3,"P_Desired_Markup",1.5,t);
		WRITELS(cur3,"P_Desired_Market_Share",norm(0.1,0.05),t);
		WRITELS(cur3, "P_Effective_Orders", 0,t); 
		WRITELS(cur3,"P_Expected_Sales",v[13],t);
		WRITELS(cur3,"P_Inventories",10,t);
		WRITELS(cur3,"P_Quality",1,t);
		WRITELS(cur3,"P_Profit_Rate",0,t);
		WRITELS(cur3,"P_Innovation_RND_Share",0.5,t);
		WRITELS(cur3,"P_Product_RND_Share",0.01,t);
		WRITELS(cur3,"P_Quality_RND_Share",0.01,t);
		WRITELS(cur3,"P_product_adjustment",0.01,t);
		WRITELS(cur3,"P_quality_adjustment",0.01,t);
		}
	
	
	
	v[14]=v[14]+1;
	}
//CYCLE_SAFE(cur2,cur4,"CAPITAL")
//	WRITES(cur4,
//	WRITES(cur4,
//	WRITES(cur4,
	
	
}
	
RESULT(v[2])
								
//EQUATION("Sector_Productive_Capacity_Entry")
/*
Sector Variable
In this variable a new firm enters if there is market space available and the entry condiion is met
*/

//   v[1]=V("Sector_Entry_Condition");									//value of entry conditions
//   v[6]=SUM("Firm_Market_Share");										//sum of firm's market share 
//      if(v[1]>0&&v[6]<1)												//if entry conditions are met and there are market space 
//     {
//	  v[20]=V("Sector_Effective_Orders");								//sector effective orders
//      v[0]=V("investment_period");										//sector investment period
//      v[22]=V("desired_degree_capacity_utilization");					//sector degree of capacity utilization
//      v[23]=V("desired_inventories_proportion");						//sector inventories proportion
//	  v[25]=V("Price_Capital_Goods");									//price of capital goods
//      v[34]=VL("Sector_Productive_Capacity_Available",1);				//productive capacity available in the last period
//	  v[44]=V("Sector_Productive_Capacity_Exit");						//productive capacity exited in the current period
//      v[36]=V("Sector_Avg_Price");										//sector avg price
//      v[35]=V("capital_output_ratio");									//sector capital output ratio
//      v[38]=V("depreciation_period");									//sector depreciation period
//      v[39]=V("Sector_Avg_Productivity");   							//sector avg productivity	
//      v[42]=uniform_int(1, v[0]);										//randon integer number between 1 and investment period
//      v[5]=COUNT("FIRMS");												//current number of firms
//     	v[8]=v[6]/v[5];													//sector simple avg market share
     	
//     	v[10]=1;														//initializes the cycle
//     	v[12]=0;														//initializes the cycle
//     	CYCLE(cur4, "FIRMS")											//CYCLE trough all firms to firm the average firm 
//     	{
//     	v[9]=VS(cur4, "Firm_Market_Share");								//current firm's market share
//        v[11]=abs(v[9]-v[8]);											//absolute difference between current firm's market share and sector's avg
//        v[10]=min(v[11],v[10]);											//min between 1 and the current difference
//            if(v[11]==v[10])											//if the current difference is the lowest one
//               v[12]=v[9];												//use current firm's market share
//            else														//if current difference is not the lowest one
//               v[12]=v[12];												//use the current value, until find the lowest difference
//     	}
		
//		cur5=SEARCH_CND("Firm_Market_Share", v[12]);					//search the firm with the closest market share to the average one
//		v[2]=VS(cur5, "Firm_Market_Share");								//market share of the average firm
//		v[3]=((v[2]*v[20])*(1+v[23]))/v[22];							//new firm's productive capacity to reach the average market share   
//		v[4]=min(v[3],(v[34]+v[44]));									//new firm's productive capacity can not be higher than productive capacity available
//		v[7]=v[4]*v[35];												//new firm's number of capitals
		
//              cur=ADDOBJ_EX("FIRMS",cur5);								//add new firm using as exemple the firm with closest market share to the average
              //begin writting some lagged variables and parameters           
//              WRITES(cur, "firm_date_birth", t);										//firm's date of birth
//              WRITES(cur, "id_firm_number",t);											//firm's number
//              WRITELS(cur, "Firm_Market_Share",v[2], t);								//firm's market share
//              WRITELS(cur, "Firm_Effective_Market_Share",v[2], t);						//firm's effective market share
//              WRITELS(cur, "Firm_Desired_Market_Share",v[2], t);						//firm's desired market share
//              WRITELS(cur, "Firm_Avg_Market_Share",v[2], t);							//firm's avg market share
//              WRITELS(cur, "Firm_Effective_Orders",(v[2]*v[20]), t);					//firm's effective orders
//              WRITELS(cur, "Firm_Stock_Inventories",(v[2]*v[20]*v[23]), t);				//firm's inventories
//              WRITELS(cur, "Firm_Price",v[36], t);										//firm's price
//              WRITELS(cur, "Firm_Max_Productivity",v[39], t);							//firm's max capital productivity
//              WRITELS(cur, "Firm_Frontier_Productivity",v[39], t);						//firm's frontier productivity
//			  WRITELS(cur, "Firm_Stock_Loans",v[7]*v[25],t);							//firm's stock of debt is the price of capital goods bought
//              WRITELS(cur, "Firm_Stock_Deposits",0,t);									//firm's stock of financial assets is zero
//              for(i=0;i<=v[0];i++)
//              	{
//				WRITELLS(cur,"Firm_Demand_Capital_Goods_Expansion", 0, t, i);
//				WRITELLS(cur,"Firm_Demand_Capital_Goods_Replacement", 0, t, i);
//				}
//			
//			  v[50]=COUNTS(cur,"CAPITALS");
//			  CYCLE_SAFES(cur, cur1, "CAPITALS")								//CYCLE trough firm's capitals
//				{
//					if(v[50]>1)
//						{
//						DELETE(cur1);												//delete the current capital
//						v[50]=v[50]-1;
//						}
//					else	
//						v[50]=v[50];
//				}
//			cur2=ADDNOBJS(cur, "CAPITALS", v[7]);
//			CYCLES(cur, cur1, "CAPITALS")
//			{
//			WRITES(cur1, "capital_good_productive_capacity",(1/v[35]));
//			WRITES(cur1, "capital_good_date_birth",t);					//write the new date of birth
//			WRITES(cur1, "capital_good_depreciation_period",(t+v[38])); //write the new date of birth
//			WRITES(cur1, "capital_good_to_replace",0);					//write current capital goods as not to replace
//			WRITES(cur1, "capital_good_productivity_initial",v[39]);	//write current capital initial productivity
//			WRITELS(cur1, "Capital_Good_Acumulated_Production",0,t);	//write current capital acumulated production as zero
//			}
		
//			v[13]=V("interest_rate_long_term");
//			CYCLE_SAFES(cur, cur1, "LOANS")
//			{
//				v[51]=V("firm_loan_fixed_object");
//				if (v[51]!=1)
//				DELETE(cur1);
//			}
			
//			cur1 = ADDOBJS(cur, "LOANS");
//			WRITES(cur1, "firm_loan_total_amount", v[7]*v[25]);
//			WRITES(cur1, "firm_loan_fixed_object", 0);
//			WRITES(cur1, "firm_loan_fixed_amortization", (v[7]*v[25]/(2*v[0])));
//			WRITES(cur1, "id_firm_loan_long_term", 1);
//			WRITES(cur1, "firm_loan_interest_rate", v[13]);
                
//     }
//   else
//  v[4]=0;
   
//	v[60]=SUM("Firm_Market_Share");												//sum of firms market share 
//	CYCLE(cur, "FIRMS")															//cycle trough firms to normalize market shares
//	{
//	v[61]=VS(cur,"Firm_Market_Share");											//current firm's market share
//	WRITES(cur, "Firm_Market_Share", (v[61]/v[60]));							//divide firm's market share by the sum of market shares
//	}
   
//RESULT(v[4])
     
     


MODELEND

// do not add Equations in this area

void close_sim( void )
{
	// close simulation special commands go here
}
