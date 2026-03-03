$base=Import-Csv 'baseline_AT_SAM.csv' -Delimiter ';'
$sec=Import-Csv 'sector_AT_SAM.csv' -Delimiter ';'
Write-Output "rows: $($base.Count) $($sec.Count)"
$base_map = @{ }
foreach ($r in $base) { $base_map[$r.Account] = $r }
$diffs=@()
foreach ($r in $sec) {
    if ($base_map.ContainsKey($r.Account)) {
        $b = [double]$base_map[$r.Account].RowSum
        $s = [double]$r.RowSum
        if ($b -ne $s) { $diffs += [PSCustomObject]@{Account=$r.Account; Base=$b; Sec=$s; Diff=$s-$b} }
    }
}
Write-Output "diff count $($diffs.Count)"
$diffs | Sort-Object @{Expression={[math]::Abs($_.Diff)};Descending=$true} | Select-Object -First 10 | Format-Table -AutoSize
