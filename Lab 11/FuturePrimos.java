/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Aluno: Rafael Oliveira Chaffin */
/* DRE: 121045260 */
/* Codigo: Contagem de primos em um intervalo usando Futures */

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import java.util.ArrayList;
import java.util.List;

//classe callable que verifica se um numero eh primo
class PrimoCallable implements Callable<Integer> {
    long n;

    // construtor
    PrimoCallable(long n) {
        this.n = n;
    }

    // método para execução
    public Integer call() throws Exception {
        if (ehPrimo(n)) {
            return 1;
        }
        return 0;
    }

    private boolean ehPrimo(long n) {
        if (n <= 1)
            return false;
        if (n == 2)
            return true;
        if (n % 2 == 0)
            return false;
        for (int i = 3; i < Math.sqrt(n) + 1; i += 2) {
            if (n % i == 0)
                return false;
        }
        return true;
    }
}

// classe do método main
public class FuturePrimos {
    private static final int NTHREADS = 8;
    private static final int MAX_NUM = 100; // Intervalo de 1 a 10000

    public static void main(String[] args) {
        // cria um pool de threads (NTHREADS)
        ExecutorService executor = Executors.newFixedThreadPool(NTHREADS);
        // cria uma lista para armazenar referencias de chamadas assincronas
        List<Future<Integer>> list = new ArrayList<Future<Integer>>();

        // submete as tarefas
        for (int i = 1; i <= MAX_NUM; i++) {
            Callable<Integer> worker = new PrimoCallable(i);
            Future<Integer> submit = executor.submit(worker);
            list.add(submit);
        }

        // recupera os resultados e faz o somatório final
        long totalPrimos = 0;
        for (Future<Integer> future : list) {
            try {
                totalPrimos += future.get(); // bloqueia se a computação nao tiver terminado
            } catch (InterruptedException e) {
                e.printStackTrace();
            } catch (ExecutionException e) {
                e.printStackTrace();
            }
        }
        System.out.println("Total de primos entre 1 e " + MAX_NUM + ": " + totalPrimos);
        executor.shutdown();
    }
}
