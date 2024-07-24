#include <Eigen/Dense>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace Eigen;

struct FinancialData {
public:
  vector<string> fund;
  vector<string> pricedate;
  vector<double> open;
  vector<double> high;
  vector<double> low;
  vector<double> close;
  vector<double> adjclose;
  vector<double> volume;

  void readCSV(const string &filename) {
    ifstream file(filename);
    string line, cell;

    if (!file.is_open()) {
      cerr << "Error opening file: " << filename << endl;
      return;
    }

    // Assuming the first line is the header
    getline(file, line);

    while (getline(file, line)) {
      stringstream lineStream(line);
      int columnIndex = 0;

      while (getline(lineStream, cell, ',')) {

        switch (columnIndex) {
        case 0: // fund
          fund.push_back(cell);
          break;
        case 1: // pricedate
          pricedate.push_back(cell);
          break;
        case 2: // open
          open.push_back(stod(cell));
          break;
        case 3: // high
          high.push_back(stod(cell));
          break;
        case 4: // low
          low.push_back(stod(cell));
          break;
        case 5: // close
          close.push_back(stod(cell));
          break;
        case 6: // adjclose
          adjclose.push_back(stod(cell));
          break;
        case 7: // volume
          volume.push_back(stod(cell));
          break;
        }
        columnIndex++;
      }
    }

    file.close();
  }
};
// change w to vector

void Coefficient_Optimizer_Matrix(const vector<double> &open,const vector<double> &high, const vector<double> &low, const vector<double> &volume, const vector<double> &close, double &w0, double &w1, double &w2, double &w3, double &w4) {
  int N = close.size();
  MatrixXd x(N, 5);
  MatrixXd y(N, 1);
  for (int i = 0; i < N; i++) {
    // features
    x(i, 0) = 1;
    x(i, 1) = open[i];
    x(i, 2) = high[i];
    x(i, 3) = low[i];
    x(i, 4) = volume[i];
    // outcome
    y(i) = close[i];
  }
  // Normal Equation to find weights
  MatrixXd w = (x.transpose() * x).inverse() * (x.transpose() * y);
  w0 = w(0);
  w1 = w(1);
  w2 = w(2);
  w3 = w(3);
  w4 = w(4);
}

void scale(vector<double> &v) {
  double max = numeric_limits<double>::lowest();
  double min = numeric_limits<double>::max();

  for (int i = 0; i < v.size(); i++) {
    if (v[i] > max) {
      max = v[i];
    }
    // scale using the smallest non zero element
    if (v[i] < min && v[i] != 0) {
      min = v[i];
    }
  }

  if (max != min) {
    double den = max - min;
    for (int i = 0; i < v.size(); i++)
      v[i] = (v[i] - min) / den;
  }
}


double error_rate(const vector<double> &predicted, const vector<double> &actual, const double &precision) {

  double num_error = 0;

  for (int i = 0; i < predicted.size(); i++) {
    if (predicted[i] < actual[i] - precision || predicted[i] > actual[i] + precision) {
      num_error++;
    }
  }
  return num_error / double(predicted.size());
}

void Predict(vector<double> &predictions, const vector<double> &close, const vector<double> &open, const vector<double> &high, const vector<double> &low, const vector<double> &volume, double w_0, double w_1, double w_2, double w_3, double w_4) {
  predictions.resize(close.size());
  for (int i = 0; i < predictions.size(); i++)
    predictions[i] = w_0 + w_1 * open[i] + w_2 * high[i] + w_3 * low[i] + w_4 * volume[i];
}
double Predict1(const double& open, const double& high, const double& low, const double& volume, const double& w_0,const  double& w_1, const double& w_2, const double& w_3, const double& w_4){
    return  w_0 + w_1 * open + w_2 * high + w_3 * low + w_4 * volume;
}
int main() {
    //data is train data
    FinancialData data;
    //rename this to train csv
    data.readCSV("training_data.csv");


    scale(data.open);
    scale(data.high);
    scale(data.low);
    scale(data.volume);
    scale(data.close);

    double w0 = 0.0;
    double w1 = 0.0;
    double w2 = 0.0;
    double w3 = 0.0;
    double w4 = 0.0;


    Coefficient_Optimizer_Matrix(data.open, data.high, data.low, data.volume, data.close, w0, w1, w2, w3, w4);


    FinancialData test;
    test.readCSV("test_data.csv");
    vector<double> predictions;
    Predict(predictions, test.close, test.open, test.high, test.low, test.volume, w0, w1, w2, w3, w4);

    double precision;
    cout << "Enter the precision: ";
    cin >> precision;

    double errorrate = error_rate(predictions, test.close, precision/2);
    cout  << " " << errorrate*100 << endl;
    cout << " Optimized w0: " << w0 << " Optimized w1: " << w1 << " Optimized w2: " << w2 << " Optimized w3: " << w3 << " Optimized w4: " << w4 << endl;

    double open, high, low, volume;
    cout<< "Market open: "<<endl;
    cin >> open;
    cout << endl;
    cout << "Market high: "<<endl;
    cin >> high;
    cout << endl;
    cout<< "Market low: "<<endl;
    cin >> low;
    cout << endl;
    cout << "volume traded: "<<endl;
    cin >> volume;
    cout << endl << "Predicted market close is  "<< Predict1(open, high, low, volume, w0, w1,w2,w3,w4) << "  +/-  "<< precision<< endl;
    cout << endl;
    
    return 0;
}
