import mac_alias
a = mac_alias.Alias.for_file('/.background/background.png')
print(a.to_bytes().hex())
