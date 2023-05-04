#ifndef SIMULATION_CLASS
#define SIMULATION_CLASS

#include <iostream>
#include <iomanip>
using namespace std;
 #pragma hdrstop

#include "random.h" // include random number generator

// specifies the two kinds of events
enum EventType {
    arrival,
    departure
};

class Event
{
private: 
    // members that identify both customer and teller, as
    // well as maintain information on time of the event,
    // the event type, the length of service required by
    // the customer, and the amount of time customer is
    // forced to wait for service
    
    // 客户数据
    int time;        // 客户到达或离开的时刻
    EventType etype; // 事件类型
    int customerID;  // 客户编号  customers numbered 1, 2, 3,...
    int tellerID;    // 出纳窗口编号 tellers numbered 1, 2, 3,...
    int waittime;    // 客户等待时间
    int servicetime; // 客户服务时间

public:
    // constructors
    Event(void);
    Event(int t, EventType et, int cn, int tn,
          int wt, int st);

    // methods to retrieve private data
    int GetTime(void) const;            // 取事件时间
    EventType GetEventType(void) const; // 取事件类型
    int GetCustomerID(void) const;      // 取客户编号
    int GetTellerID(void) const;        // 取窗口编号
    int GetWaitTime(void) const;        // 取等待时间
    int GetServiceTime(void) const;     // 取服务时间
};

// default constructor - data filled by assignment later
Event::Event(void)
{
}

// constructor that initializes all data members of the event
Event::Event(int t, EventType et, int cn, int tn,
             int wt, int st) : time(t), etype(et), customerID(cn), tellerID(tn),
                               waittime(wt), servicetime(st)
{
}

// return the time the event occurs
int Event::GetTime(void) const
{
    return time;
}

// return the type of event (arrival, departure)
EventType Event::GetEventType(void) const
{
    return etype;
}

// return the customer number
int Event::GetCustomerID(void) const
{
    return customerID;
}

// return the teller number
int Event::GetTellerID(void) const
{
    return tellerID;
}

// return the time the customer waits for service
int Event::GetWaitTime(void) const
{
    return waittime;
}

// return the amount of teller time needed by customer
int Event::GetServiceTime(void) const
{
    return servicetime;
}

// compare two Event objects using the time at
// which the events occur. needed for the priority queue
int operator<(Event e1, Event e2)
{
    return e1.GetTime() < e2.GetTime();
}

typedef Event DataType; // elements are Event objects

#include "apqueue.h"

// Structure for Teller Info
struct TellerStats          // 出纳窗口信息
{
    int finishService;      // 空闲时刻预告 when teller available
    int totalCustomerCount; // 服务过的客户总数 total of customers serviced
    int totalCustomerWait;  // 客户总的等待时间 total customer waiting time
    int totalService;       // 总的服务时间 total time servicing customers
};

class Simulation                 // 模拟类说明
{
private:
    // data used to run the simulation
    int simulationLength;        // 模拟时间长度 simulation length
    int numTellers;              // 出纳窗口个数 number of tellers
    int nextCustomer;            // 下一位客户的编号 next customer ID
    int arrivalLow, arrivalHigh; // 到达时间范围限制 next arrival range
    int serviceLow, serviceHigh; // 服务时间范围限制 service range
    TellerStats tstat[11];       // 最多10个出纳窗口 max 10 tellers
    PQueue pq;                   // 事件队列（优先级队列） priority queue
    RandomNumber rnd;            // 随机数用于产生到达和服务时间 use for arrival
                                 // and service times

    // private methods used by RunSimulation
    int NextArrivalTime(void);     // 私有函数：产生下一个客户的到达时刻
    int GetServiceTime(void);      // 私有函数：产生客户需要的服务时间
    int NextAvailableTeller(void); // 私有函数：选择一个窗口

public:
    // constructor
    Simulation(void);                  // 构造函数

    void RunSimulation(void);          // 执行模拟 execute study
    void PrintSimulationResults(void); // 输出模拟结果 print stats
};

// constructor initializes simulation data and prompts client
// for simulation parameters
Simulation::Simulation(void)
{
    int i;
    Event firstevent;              // 第一个到达事件

    // Initialize Teller Information Parameters
    for (i = 1; i <= 10; i++)
    {
        tstat[i].finishService = 0;
        tstat[i].totalService = 0;
        tstat[i].totalCustomerWait = 0;
        tstat[i].totalCustomerCount = 0;
    }
    nextCustomer = 1;                                        // 客户编号从1开始

    // reads client input for the study
    cout << "Enter the simulation time in minutes: ";
    cin >> simulationLength;                                 // 输入模拟时间长度
    cout << "Enter the number of bank tellers: ";
    cin >> numTellers;                                       // 输入出纳窗口个数
    cout << "Enter the range of arrival times in minutes: ";
    cin >> arrivalLow >> arrivalHigh;                        // 输入到达时间范围
    cout << "Enter the range of service times in minutes: ";
    cin >> serviceLow >> serviceHigh;                        // 输入服务时间范围

    // generate first arrival event
    // teller#/waittime/servicetime not used for arrival
    pq.PQInsert(Event(0, arrival, 1, 0, 0, 0));              // 第一个到达事件入队
}

// 成员函数：确定下一个客户到达的随机时刻 determine random time of next arrival
int Simulation::NextArrivalTime(void)
{
    return arrivalLow + rnd.Random(arrivalHigh - arrivalLow + 1);
}

// 成员函数：确定客户服务的随机时间 determine random time for customer service
int Simulation::GetServiceTime(void)
{
    return serviceLow + rnd.Random(serviceHigh - serviceLow + 1);
}

// 成员函数：选择下一个可用窗口 return first available teller
int Simulation::NextAvailableTeller(void)
{
    // 初始假定所有窗口在下班时关闭 initially assume all tellers finish at closing time
    int minfinish = simulationLength;

    // assign random teller to customer who arrives
    // before closing but obtains service after closing
    // 给下班前到达但在下班后得到服务的客户提供一个随机的窗口编号
    int minfinishindex = rnd.Random(numTellers) + 1;

    // 找一个可用的窗口 find teller who is free first
    // 寻找窗口空闲时刻最小者
    for (int i = 1; i <= numTellers; i++)
        if (tstat[i].finishService < minfinish)
        {
            minfinish = tstat[i].finishService;
            minfinishindex = i;
        }
    // 返回窗口空闲时刻最小者的窗口号码
    return minfinishindex;
}

// 模拟主函数 implements the simulation
void Simulation::RunSimulation(void)
{
    Event e, newevent;
    int nexttime;
    int tellerID;
    int servicetime;
    int waittime;

    // 模拟一直进行到队列为空时停止 run till priority queue is empty
    while (!pq.PQEmpty())
    {
        // 取一个事件 get next event (time measures the priority)
        e = pq.PQDelete();

        // 到达事件的处理 handle an arrival event
        if (e.GetEventType() == arrival)
        {
            // compute time for next arrival.
            nexttime = e.GetTime() + NextArrivalTime();
            // 下一个客户到达时已下班
            if (nexttime > simulationLength)
                // process events but don't generate any more
                continue;
            else
            {
                // 下一个客户编号 generate arrival for next customer. put in queue
                nextCustomer++;
                // 产生下一个客户到达事件
                newevent = Event(nexttime, arrival,
                                 nextCustomer, 0, 0, 0);
                // 下一个客户到达事件入队
                pq.PQInsert(newevent);
            }

            cout << "Time: " << setw(2) << e.GetTime()
                 << "  "
                 << "arrival of customer "
                 << e.GetCustomerID() << endl;

            // generate departure event for current customer

            // time the customer takes
            servicetime = GetServiceTime();
            // teller who services customer
            tellerID = NextAvailableTeller();

            // if teller free, update sign to current time
            if (tstat[tellerID].finishService == 0)
                tstat[tellerID].finishService = e.GetTime();

            // compute time customer waits by subtracting the
            // current time from time on the teller's sign
            waittime = tstat[tellerID].finishService -
                       e.GetTime();

            // update teller statistics
            tstat[tellerID].totalCustomerWait += waittime;
            tstat[tellerID].totalCustomerCount++;
            tstat[tellerID].totalService += servicetime;
            tstat[tellerID].finishService += servicetime;

            // 产生下一个客户离开事件 create a departure object and put in the queue
            newevent = Event(tstat[tellerID].finishService,
                             departure, e.GetCustomerID(), tellerID,
                             waittime, servicetime);
            // 下一个客户离开事件入队
            pq.PQInsert(newevent);
        }
        // handle a departure event
        else
        {
            cout << "Time: " << setw(2) << e.GetTime()
                 << "  "
                 << "departure of customer "
                 << e.GetCustomerID() << endl;
            cout << "       Teller " << e.GetTellerID()
                 << "  Wait " << e.GetWaitTime()
                 << "  Service " << e.GetServiceTime()
                 << endl;
            tellerID = e.GetTellerID();
            // if nobody waiting for teller, mark teller free
            if (e.GetTime() == tstat[tellerID].finishService)
                tstat[tellerID].finishService = 0;
        }
    }

    // 最后一个离开事件若发生在下班之后，则调整模拟时间长度 adjust simulation to account for overtime by tellers
    simulationLength = (e.GetTime() <= simulationLength)
                           ? simulationLength
                           : e.GetTime();
}

// 模拟输出结果 summarize the simulation results
void Simulation::PrintSimulationResults(void)
{
    int cumCustomers = 0, cumWait = 0, i;
    int avgCustWait, tellerWorkPercent;
    float tellerWork;

    for (i = 1; i <= numTellers; i++)
    {
        cumCustomers += tstat[i].totalCustomerCount;
        cumWait += tstat[i].totalCustomerWait;
    }
    // 打印模拟时间长度
    cout << endl;
    cout << "******** Simulation Summary ********" << endl;
    cout << "Simulation of " << simulationLength
         << " minutes" << endl;
    // 打印客户总数
    cout << "   No. of Customers:  " << cumCustomers << endl;
    cout << "   Average Customer Wait: ";
    // 计算平均等待时间（舍入后）
    avgCustWait = int(float(cumWait) / cumCustomers + 0.5);
    // 打印平均等待时间
    cout << avgCustWait << " minutes" << endl;
    for (i = 1; i <= numTellers; i++)
    {
        cout << "    Teller #" << i << "  % Working: ";
        // 计算窗口平均服务时间 display percent rounded to nearest integer value
        tellerWork = float(tstat[i].totalService) / simulationLength;
        // 服务时间百分比（舍入后）
        tellerWorkPercent = int(tellerWork * 100.0 + 0.5);
        cout << tellerWorkPercent << endl;
    }
}

#endif // SIMULATION_CLASS
