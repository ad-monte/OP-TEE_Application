__Ideas para Trusted App__

1. ta_secret -e test.txt key pwd //encrypt Hardcoded
2. ta_secret -d key pwd //decrypt Hardcoded
3. ta_secret -a num pwd //Acceso a access log
    
    -Verificar PWD (SebasRico) 
    -text.txt: {PlainText} 
    -ciphertext.bin:{PlainText}




__Ataques__
1. PWD: Timing analysis en la comparación
2. Key: Leaked secret between sessions
3. Mapear memoria: Lectura fuera de límites
4. Race condition: Dos sesiones abiertas simultáneamente modifican un objeto persistente (¿access log?)

__Temas pendientes__
1. Investigar (deá de invetnar mierda) |
- Verificar si dos sesiones pueden acceder a un objeto persistente y modificarlo, provocando una race condition
    - Understanding TEE memory management and its security aspects
- Investigar sobre timing análisis y sobre qué se puede aplicar