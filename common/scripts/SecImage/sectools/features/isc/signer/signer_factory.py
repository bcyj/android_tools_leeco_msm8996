class SignerFactory(object):
    @classmethod
    def getSigner(cls, selected_signer, config):
        if selected_signer == "cass":
            import sectools.features.isc.signer.cass_signer as cass_signer
            signer = cass_signer.CassSigner(config)
        elif selected_signer == "csms":
            import csms_signer
            signer = csms_signer.CsmsSigner(config)
        elif selected_signer == "local":
            import openssl_signer
            signer = openssl_signer.OpenSSLSigner(config)
        else:
            raise RuntimeError, "Unsupported signer = {0}".format(selected_signer)

        return signer
