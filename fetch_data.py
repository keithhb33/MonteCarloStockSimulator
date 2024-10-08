import sys
import yfinance as yf
from datetime import datetime, timedelta

def fetch_data(ticker, start_date=None, end_date=None):
    if start_date and end_date:
        # Parse dates
        try:
            start = datetime.strptime(start_date, '%Y-%m-%d')
            end = datetime.strptime(end_date, '%Y-%m-%d')
        except ValueError:
            print(f"Error: Invalid date format. Use YYYY-MM-DD.")
            sys.exit(1)

        # Ensure start date is before end date
        if start >= end:
            print(f"Error: Start date {start_date} must be before end date {end_date}.")
            sys.exit(1)

        # Adjust end date to include the end date (yfinance end date is exclusive)
        adjusted_end = end + timedelta(days=1)
        adjusted_end_str = adjusted_end.strftime('%Y-%m-%d')

        data = yf.download(ticker, start=start_date, end=adjusted_end_str, interval='1d')
    else:
        data = yf.download(ticker, period='max', interval='1d')

    if data.empty:
        print(f"Error: No data fetched for {ticker} from {start_date} to {end_date}.")
        sys.exit(1)
    else:
        data.to_csv('stock_data.csv')
        print("Data fetched successfully.")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python fetch_data.py <ticker> [<start_date> <end_date>]")
        sys.exit(1)

    ticker = sys.argv[1]
    if len(sys.argv) == 4:
        start_date = sys.argv[2]
        end_date = sys.argv[3]
        fetch_data(ticker, start_date, end_date)
    else:
        fetch_data(ticker)
