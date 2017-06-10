#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

import openssl_implementation_discovery
import utility_functions


def gen_rsa_key_pair(key_size_in_bits, key_exponent = 3, priv_key_output_file = None, pub_key_output_file = None):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.gen_rsa_key_pair(key_size_in_bits = key_size_in_bits, key_exponent = key_exponent, priv_key_output_file = priv_key_output_file, pub_key_output_file=pub_key_output_file)

def generate_hash(hashing_algorithm, file_to_hash):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.generate_hash(hashing_algorithm, file_to_hash)

def privkey_der_to_pem(der_privkey):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.privkey_der_to_pem(der_privkey)

def privkey_pem_to_der(pem_privkey):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.privkey_pem_to_der(pem_privkey)

def cert_der_to_pem(der_certificate):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.cert_der_to_pem(der_certificate)

def cert_pem_to_der(pem_certificate):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.cert_pem_to_der(pem_certificate)

def encrypt_with_private_key(message, private_key):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.encrypt_with_private_key(message, private_key)

def decrypt_with_public_key(encypted_message, public_key):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.decrypt_with_public_key(encypted_message, public_key)

def decrypt_with_private_der_key(encrypted_message, private_key):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.decrypt_with_private_der_key(encrypted_message, private_key)

def create_root_certficate(root_cert_params, root_key_pair, days, configfile, serial_num):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.create_root_certficate(root_cert_params, root_key_pair, days, configfile, serial_num)

def create_certificate(certificate_params, certificate_key_pair, CACertificate, CA_key_pair, days=7300, configfile="opensslroot.cfg", serial_num=1, extfile_name="v3.ext"):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.create_certificate(certificate_params, certificate_key_pair, CACertificate, CA_key_pair, days, configfile, serial_num, extfile_name)

def get_public_key_from_cert_chain(cert_chain_list):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.get_public_key_from_cert_chain(cert_chain_list)

def verify_certificate_chain(certificate_chain):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.verify_certificate_chain(certificate_chain)

def create_certificate_chain(certificate_list):
    certificate_chain="".join(certificate_list)
    return certificate_chain

def split_certificate_blob_into_certs(certificate_blob):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.split_certificate_blob_into_certs(certificate_blob)

def get_der_certificate_text(der_certificate_path):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.get_der_certificate_text(der_certificate_path)

def get_certificate_text_from_binary(certificate_blob):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.get_certificate_text_from_binary(certificate_blob)

def get_asn1_text(pem_certificate_path):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.get_asn1_text(pem_certificate_path)

def get_asn1_text_from_binary(certificate_blob):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.get_asn1_text_from_binary(certificate_blob)

def get_hmacparams_from_certificate_chain(certificate_chain_blob):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.get_hmacparams_from_certificate_chain(certificate_chain_blob)

def get_public_key_from_private_key(private_key):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.get_public_key_from_private_key(private_key)

def cbc_cts_encrypt_binary(binary_blob, preexisting_128_bit_key, preexisting_iv):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.cbc_cts_encrypt_binary(binary_blob, preexisting_128_bit_key, preexisting_iv)

def cbc_cts_decrypt_binary(encrypted_blob, preexisting_128_bit_key, preexisting_iv):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.cbc_cts_decrypt_binary(encrypted_blob, preexisting_128_bit_key, preexisting_iv)

def cbc_encrypt_binary(binary_blob, preexisting_128_bit_key, preexisting_iv):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.cbc_encrypt_binary(binary_blob, preexisting_128_bit_key, preexisting_iv)

def cbc_decrypt_binary(encrypted_blob, preexisting_128_bit_key, preexisting_iv):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.cbc_decrypt_binary(encrypted_blob, preexisting_128_bit_key, preexisting_iv)

def ccm_encrypt_binary(binary_blob, hex_preexisting_128_bit_key, hex_preexisting_iv, hex_preexisting_aad):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.ccm_encrypt_binary(binary_blob, hex_preexisting_128_bit_key, hex_preexisting_iv, hex_preexisting_aad)

def ccm_decrypt_binary(encrypted_blob, hex_preexisting_128_bit_key, hex_preexisting_iv, hex_preexisting_aad):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.ccm_decrypt_binary(encrypted_blob, hex_preexisting_128_bit_key, hex_preexisting_iv, hex_preexisting_aad)

def get_public_key_from_certificate(certificate):
    openssl_service = openssl_implementation_discovery.OpenSSL_Service()
    return openssl_service.openssl_implementation.get_public_key_from_certificate(certificate)

if __name__=='__main__':
    certificate = utility_functions.get_data_from_file('/local/mnt/workspace/vikramn/Code/sectools/sectools/features/isc/test/resources/9x35/mbn80/attestation.crt')
    get_public_key_from_certificate(certificate)
