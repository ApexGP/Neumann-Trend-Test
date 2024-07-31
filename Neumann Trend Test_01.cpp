#include <iostream>
#include <cmath>
#include <iomanip>
#include <cstring> 

using namespace std;

int main () 
{
    int x[1000], y[1000], n;//�������ͱ��������� ��Define integer variables and arrays.
    float p[1000], PG[1000];//���帡���� �� Define floats.
    char s[1000][10], t[1000][10], r[1000][10], o[1000][10];
    char str1[10] = "yes", str2[10] = "no", str3[10] = "--";//�����ַ����� ��Define character arrays.
    
    cout << "Number of data sets: ";
    cin >> n;
    
    strcpy(o[0], str3); 
    strcpy(o[1], str3);
    strcpy(o[2], str3);//ǰ3�����ݵ�Ŧ�������޷��ؽ�� ��Ensure consistent indexing with 0-based arrays
    p[0] = 0.7805;
    p[1] = 0.8204;
    p[2] = 0.8902;
    p[3] = 0.9359;//p������ΪŦ�����Ʊ�׼ֵ���룻Assign p as Neumann Trend Standard W(P).
    
    cout << "Time(h): ";
    for (int a = 0; a < n; ++a) 
	{
        cin >> y[a];
        sprintf(t[a], "%d", y[a]);//�������ʱ������ ��Enter the time of measurement for each.
    } 
    
    cout << "Area: ";
    for (int a = 0; a < n; ++a) 
	{
        cin >> x[a];
        sprintf(s[a], "%d", x[a]);//�����װ������� ��Enter the encapsulation area for each.
    }

    cout << "Neumann Trend Test:" << endl;
    cout << "--------------------------------------------------------" << endl;
    cout << setiosflags(ios::left) << setw(14) << "Area" 
         << resetiosflags(ios::left) << setiosflags(ios::right) 
         << setw(9) << "Time(h)" << setw(14) << "PG" << setw(18) << "Trend��Y/N��"
         << resetiosflags(ios::right) << endl;
    cout << "--------------------------------------------------------" << endl;

    for (int i = 3; i <= n - 1; ++i) //ȡÿ(i+1)�����ݵ����һ��Ŧ�����ԣ���(n-3)�� ��There are (n-3) testing groups in total and for each group algorithm processes (i+1) data points for caculation. 
	{
        int sum0 = 0, sum1 = 0;
        double sum2=0, x0 = 0;//���ϲ�forѭ��������ʼ�� ��Initialize variables before each prior loop conducted. 
        
        for (int j = 0; j <= i; ++j) 
		{
            sum0 += x[j];// ��ÿ(i+1)����װ�������һ����� ��Return sum0 for every (i+1) encapsulation areas in a group. 
        }
        x0 = static_cast<float>(sum0) / double(i + 1);//����x0��Ϊÿ(i+1)����װ�����ƽ��ֵ ��Return x0 for an average of sum0 and break the secondary loop.

        for (int k = 0; k <= i; ++k) 
		{
            if (k <= i-1) 
			{
                sum1 += pow((x[k] - x[k + 1]), 2);
            }
            sum2 += pow((x[k] - x0), 2);//���չ�ʽ PG=(��(x_i-x_(i+1) )^2 )/(��(x_i-x0 )^2 )������sum1��Ϊ���ӣ�sum2��Ϊ��ĸ ��Return sum1 for numerator and sum2 for denominator in accordance with the Neumann Formula.  
        }
        PG[i] = static_cast<float>(sum1) / sum2;//����PG��Ϊÿ(i+1)�����ݵ��Ŧ������ �� Return PG for Neumann Trend for every (i+1) data points.
        sprintf(r[i], "%4f", PG[i]);
    }

    for (int a = 0; a < n; ++a) 
	{
        if (a >= 3) 
		{
            if (PG[a] <= p[a - 3])//����Ŧ�����Ա�׼����95%���Ÿ����£�ȡ��׼Ŧ������P(a+3)=W(P)_n��Ϊ�ж����ݣ����PG����P����˵��û���½����� ��Accoring to Table value of Standard W(P) for Neumann Trend Test, we take W(P)_(a+3) for basis of judgment under 95% fiducial probability.If PG>P, there is no trend.Otherwise, there is a potential trend for degradation.
                strcpy(o[a], str1);
            else
                strcpy(o[a], str2);
        }
        cout << setiosflags(ios::left) << setw(14) << s[a] 
             << resetiosflags(ios::left) << setiosflags(ios::right) 
             << setw(9) << t[a] << setw(14) << r[a] << setw(18) << o[a]
             << resetiosflags(ios::right) << endl;
    }

    cout << "--------------------------------------------------------" << endl;
    return 0;
}
 
 //6147338 6147785 6154157 6141375 6127575 6118354 6090916 ; 0 1 2 5 10 18 24
 //6126811 6120857 6112046 6100250 6089047 6070963 6057447
 //7582457 7586934 7570156 7553261 7549265 ; 0 2 5 10 24
 //7342482 7311815 7308719 7282647 7283729
