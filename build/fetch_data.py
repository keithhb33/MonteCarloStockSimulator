import sys
import yfinance as yf

def fetch_data(ticker):
    data = yf.download(ticker, period='max', interval='1d')
    if data.empty:
        print("Error: No data fetched.")
        sys.exit(1)
    else:
        data.to_csv('stock_data.csv')
        print("Data fetched successfully.")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python fetch_data.py <ticker>")
        sys.exit(1)

    ticker = sys.argv[1]
    fetch_data(ticker)
