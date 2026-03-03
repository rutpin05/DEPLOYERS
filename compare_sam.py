import csv
base = list(csv.DictReader(open('baseline_AT_SAM.csv'), delimiter=';'))
sec = list(csv.DictReader(open('sector_AT_SAM.csv'), delimiter=';'))
print('rows:', len(base), len(sec))
base_map = {r['Account']:r for r in base}
sec_map = {r['Account']:r for r in sec}
common = set(base_map.keys()) & set(sec_map.keys())
diffs=[]
for acct in common:
    b=float(base_map[acct]['RowSum'])
    s=float(sec_map[acct]['RowSum'])
    if b!=s:
        diffs.append((acct,b,s,s-b))
diffs.sort(key=lambda x:abs(x[3]), reverse=True)
print('common accounts', len(common))
print('diff count', len(diffs))
for d in diffs[:10]:
    print(d)
