#!/usr/bin/env python3
"""
BLE Test Analysis Script
Compares GDP and unemployment across 4 test scenarios:
1. Baseline (no BLE, no pandemic)
2. BLE Normal (BLE enabled, no pandemic)
3. Pandemic Naive (no BLE, pandemic)
4. Pandemic + BLE (BLE enabled, pandemic)
"""

import re
import os
from pathlib import Path

def parse_curves_from_log(log_file: str) -> dict:
    """Extract [CURVES] data from log file."""
    data = {"months": [], "production": [], "unemployment": [], "realGDP": []}
    
    with open(log_file, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    # Pattern: [CURVES] Month NNN, Production=X.XX, Unempl%=Y.YY, realGDP=Z.ZZ
    pattern = r'\[CURVES\]\s*Month\s*(\d+),\s*Production=([\d.]+),\s*Unempl%=([\d.]+),\s*realGDP=([\d.]+)'
    matches = re.findall(pattern, content)
    
    for match in matches:
        month, prod, unemp, gdp = match
        data["months"].append(int(month))
        data["production"].append(float(prod))
        data["unemployment"].append(float(unemp))
        data["realGDP"].append(float(gdp))
    
    return data

def extract_ble_expectations(log_file: str) -> list:
    """Extract BLE expectations from log file."""
    ble_data = []
    
    with open(log_file, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    # Pattern: [BLE] Month NNN: ExpGrowth=X.X%, ExpInflation=Y.Y%, alpha_Y=Z.Z, beta_Y=W.W
    pattern = r'\[BLE\]\s*Month\s*(\d+):\s*ExpGrowth=([-\d.]+)%,\s*ExpInflation=([-\d.]+)%'
    matches = re.findall(pattern, content)
    
    for match in matches:
        month, growth, inflation = match
        ble_data.append({
            "month": int(month),
            "expected_growth": float(growth),
            "expected_inflation": float(inflation)
        })
    
    return ble_data

def find_log_file(test_name: str) -> str:
    """Find the main log file for a test."""
    candidates = [
        f"{test_name}_AT_Log.dep",
        f"{test_name}_Log.dep"
    ]
    for candidate in candidates:
        if os.path.exists(candidate):
            return candidate
    return None

def main():
    tests = {
        "test_1_baseline": "Baseline (No BLE, No Pandemic)",
        "test_2_ble_normal": "BLE Normal (BLE, No Pandemic)",
        "test_3_pandemic_naive": "Pandemic Naive (No BLE, Pandemic)",
        "test_4_pandemic_ble": "Pandemic + BLE (BLE + Pandemic)"
    }
    
    results = {}
    
    print("=" * 70)
    print("BLE Implementation Test Results")
    print("=" * 70)
    
    for test_name, description in tests.items():
        log_file = find_log_file(test_name)
        if not log_file:
            print(f"\nWARNING: Log file not found for {test_name}")
            continue
            
        print(f"\n{description}")
        print("-" * 50)
        
        data = parse_curves_from_log(log_file)
        results[test_name] = data
        
        if not data["months"]:
            print("  No data found!")
            continue
        
        # Summary statistics
        max_month = max(data["months"])
        
        # Find key metrics at specific months
        calibration_end = 156
        pandemic_start = 180  # Approx
        
        def get_value_at_month(metric, target_month):
            for i, m in enumerate(data["months"]):
                if m == target_month:
                    return data[metric][i]
            return None
        
        # Post-calibration average (month 157-180)
        post_cal_gdp = [data["realGDP"][i] for i, m in enumerate(data["months"]) if 157 <= m <= 180]
        post_cal_unemp = [data["unemployment"][i] for i, m in enumerate(data["months"]) if 157 <= m <= 180]
        
        # Post-pandemic average (month 181-200) if applicable
        post_pandemic_gdp = [data["realGDP"][i] for i, m in enumerate(data["months"]) if 181 <= m <= 200]
        post_pandemic_unemp = [data["unemployment"][i] for i, m in enumerate(data["months"]) if 181 <= m <= 200]
        
        print(f"  Months simulated: 0 to {max_month}")
        
        if post_cal_gdp:
            avg_gdp_pre = sum(post_cal_gdp) / len(post_cal_gdp)
            avg_unemp_pre = sum(post_cal_unemp) / len(post_cal_unemp)
            print(f"  Avg GDP (months 157-180): {avg_gdp_pre:.2f}")
            print(f"  Avg Unemployment (months 157-180): {avg_unemp_pre:.2f}%")
        
        if post_pandemic_gdp:
            avg_gdp_post = sum(post_pandemic_gdp) / len(post_pandemic_gdp)
            avg_unemp_post = sum(post_pandemic_unemp) / len(post_pandemic_unemp)
            print(f"  Avg GDP (months 181-200): {avg_gdp_post:.2f}")
            print(f"  Avg Unemployment (months 181-200): {avg_unemp_post:.2f}%")
            
            if post_cal_gdp:
                gdp_change = ((avg_gdp_post - avg_gdp_pre) / avg_gdp_pre) * 100
                print(f"  GDP change from pre to post: {gdp_change:+.1f}%")
        
        # BLE expectations if available
        if "ble" in test_name:
            ble_data = extract_ble_expectations(log_file)
            if ble_data:
                print(f"  BLE expectations logged: {len(ble_data)} entries")
                # Show last few
                for entry in ble_data[-3:]:
                    print(f"    Month {entry['month']}: Growth={entry['expected_growth']:+.1f}%, Inflation={entry['expected_inflation']:.1f}%")
    
    # Comparison summary
    print("\n" + "=" * 70)
    print("COMPARISON SUMMARY")
    print("=" * 70)
    
    if "test_1_baseline" in results and "test_2_ble_normal" in results:
        b1 = results["test_1_baseline"]
        b2 = results["test_2_ble_normal"]
        
        # Compare GDP volatility (std dev) in post-calibration period
        gdp1 = [b1["realGDP"][i] for i, m in enumerate(b1["months"]) if 157 <= m <= 200]
        gdp2 = [b2["realGDP"][i] for i, m in enumerate(b2["months"]) if 157 <= m <= 200]
        
        if gdp1 and gdp2:
            import statistics
            std1 = statistics.stdev(gdp1) if len(gdp1) > 1 else 0
            std2 = statistics.stdev(gdp2) if len(gdp2) > 1 else 0
            print(f"\nGDP Volatility (std dev, months 157-200):")
            print(f"  Baseline (no BLE): {std1:.2f}")
            print(f"  BLE Normal:        {std2:.2f}")
            print(f"  BLE {'reduces' if std2 < std1 else 'increases'} volatility by {abs(std2-std1)/std1*100:.1f}%")
    
    if "test_3_pandemic_naive" in results and "test_4_pandemic_ble" in results:
        p3 = results["test_3_pandemic_naive"]
        p4 = results["test_4_pandemic_ble"]
        
        # Compare pandemic impact
        gdp3_pre = [p3["realGDP"][i] for i, m in enumerate(p3["months"]) if 157 <= m <= 180]
        gdp3_post = [p3["realGDP"][i] for i, m in enumerate(p3["months"]) if 181 <= m <= 200]
        gdp4_pre = [p4["realGDP"][i] for i, m in enumerate(p4["months"]) if 157 <= m <= 180]
        gdp4_post = [p4["realGDP"][i] for i, m in enumerate(p4["months"]) if 181 <= m <= 200]
        
        if gdp3_pre and gdp3_post and gdp4_pre and gdp4_post:
            avg3_pre = sum(gdp3_pre) / len(gdp3_pre)
            avg3_post = sum(gdp3_post) / len(gdp3_post)
            avg4_pre = sum(gdp4_pre) / len(gdp4_pre)
            avg4_post = sum(gdp4_post) / len(gdp4_post)
            
            drop3 = (avg3_post - avg3_pre) / avg3_pre * 100
            drop4 = (avg4_post - avg4_pre) / avg4_pre * 100
            
            print(f"\nPandemic GDP Impact:")
            print(f"  Naive (no BLE): {drop3:+.1f}% change")
            print(f"  With BLE:       {drop4:+.1f}% change")
            print(f"  BLE {'amplifies' if abs(drop4) > abs(drop3) else 'dampens'} pandemic impact")
    
    print("\n" + "=" * 70)
    print("Analysis complete!")

if __name__ == "__main__":
    main()
